#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QElapsedTimer>
#include <QApplication>
#include <QPalette>
#include <QTextStream>
#include <QFile>
#include <algorithm>
#include <cctype>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    engine = new SearchEngine(&searchTree);
    setupUi();
    setWindowTitle("Local Search Assistant");
    resize(900, 600);
}

MainWindow::~MainWindow()
{
    delete engine;
}

// ---------------------------------------------------------------------
// UI construction
// ---------------------------------------------------------------------
void MainWindow::setupUi()
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // ---- Top bar: directory + scan controls ----
    QHBoxLayout *topBar = new QHBoxLayout();
    dirPathEdit = new QLineEdit(this);
    dirPathEdit->setPlaceholderText("Choose a folder to index...");
    browseButton = new QPushButton("Browse...", this);
    recursiveCheck = new QCheckBox("Include subfolders", this);
    recursiveCheck->setChecked(true);
    scanButton = new QPushButton("Scan && Index", this);
    darkModeCheck = new QCheckBox("Dark mode", this);

    topBar->addWidget(dirPathEdit, 1);
    topBar->addWidget(browseButton);
    topBar->addWidget(recursiveCheck);
    topBar->addWidget(scanButton);
    topBar->addWidget(darkModeCheck);

    mainLayout->addLayout(topBar);

    // ---- Tabs ----
    tabs = new QTabWidget(this);

    // --- Search tab ---
    QWidget *searchTab = new QWidget(this);
    QVBoxLayout *searchLayout = new QVBoxLayout(searchTab);

    QHBoxLayout *searchBar = new QHBoxLayout();
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Search by name, prefix, or extension (e.g. .pdf)...");
    searchModeCombo = new QComboBox(this);
    searchModeCombo->addItems({"Smart Search", "Exact Search", "Prefix Search", "Extension Search"});
    exportButton = new QPushButton("Export Results", this);

    searchBar->addWidget(searchEdit, 1);
    searchBar->addWidget(searchModeCombo);
    searchBar->addWidget(exportButton);

    resultsList = new QListWidget(this);
    resultsList->setAlternatingRowColors(true);

    searchLayout->addLayout(searchBar);
    searchLayout->addWidget(resultsList);

    tabs->addTab(searchTab, "Search");

    // --- Duplicates tab ---
    QWidget *dupTab = new QWidget(this);
    QVBoxLayout *dupLayout = new QVBoxLayout(dupTab);

    findDuplicatesButton = new QPushButton("Find Duplicate Files", this);
    duplicatesTree = new QTreeWidget(this);
    duplicatesTree->setColumnCount(3);
    duplicatesTree->setHeaderLabels({"File", "Size", "Path"});
    duplicatesTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    dupLayout->addWidget(findDuplicatesButton);
    dupLayout->addWidget(duplicatesTree);

    tabs->addTab(dupTab, "Duplicate Finder");

    mainLayout->addWidget(tabs, 1);

    // ---- Status bar ----
    statusLabel = new QLabel("Ready. Choose a folder and click \"Scan && Index\" to begin.", this);
    mainLayout->addWidget(statusLabel);

    setCentralWidget(central);

    // ---- Signals ----
    connect(browseButton, &QPushButton::clicked, this, &MainWindow::browseDirectory);
    connect(scanButton, &QPushButton::clicked, this, &MainWindow::scanAndIndex);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::performSearch);
    connect(searchModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::searchModeChanged);
    connect(findDuplicatesButton, &QPushButton::clicked, this, &MainWindow::findDuplicates);
    connect(resultsList, &QListWidget::itemDoubleClicked, this, &MainWindow::onResultDoubleClicked);
    connect(duplicatesTree, &QTreeWidget::itemDoubleClicked, this, &MainWindow::onDuplicateDoubleClicked);
    connect(darkModeCheck, &QCheckBox::toggled, this, &MainWindow::toggleDarkMode);
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportResults);
}

// ---------------------------------------------------------------------
// Directory selection + indexing
// ---------------------------------------------------------------------
void MainWindow::browseDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Folder to Index",
                                                      QDir::homePath());
    if (!dir.isEmpty())
    {
        dirPathEdit->setText(dir);
    }
}

void MainWindow::scanAndIndex()
{
    QString dir = dirPathEdit->text().trimmed();
    if (dir.isEmpty() || !QFileInfo(dir).isDir())
    {
        QMessageBox::warning(this, "Invalid Folder", "Please choose a valid folder first.");
        return;
    }

    statusLabel->setText("Scanning...");
    QApplication::processEvents();

    QElapsedTimer timer;
    timer.start();

    try
    {
        std::string path = dir.toStdString();
        bool recursive = recursiveCheck->isChecked();

        std::vector<std::string> files = scanner.scanFiles(path, recursive);
        indexedFiles = indexer.indexFiles(files, path);

        // Rebuild the Trie and the path->metadata lookup from scratch
        // every time we (re)index, so results always match the folder
        // currently on screen.
        searchTree = Trie();
        delete engine;
        engine = new SearchEngine(&searchTree);
        pathToInfo.clear();

        for (const FileInfo &file : indexedFiles)
        {
            std::string lowerName = file.name;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                            [](unsigned char c) { return std::tolower(c); });

            searchTree.insert(lowerName, file.path);
            pathToInfo.insert(QString::fromStdString(file.path), file);
        }

        qint64 ms = timer.elapsed();
        statusLabel->setText(QString("Indexed %1 file(s) from \"%2\" in %3.")
                                  .arg(indexedFiles.size())
                                  .arg(dir)
                                  .arg(elapsedString(ms)));

        resultsList->clear();
        duplicatesTree->clear();

        if (!searchEdit->text().isEmpty())
        {
            performSearch(searchEdit->text());
        }
    }
    catch (const std::exception &ex)
    {
        QMessageBox::critical(this, "Indexing Error", ex.what());
        statusLabel->setText("Indexing failed.");
    }
}

// ---------------------------------------------------------------------
// Search
// ---------------------------------------------------------------------
void MainWindow::searchModeChanged(int)
{
    if (!searchEdit->text().isEmpty())
    {
        performSearch(searchEdit->text());
    }
}

void MainWindow::performSearch(const QString &text)
{
    if (indexedFiles.empty())
    {
        showEmptyState("Index a folder first (top-left \"Scan && Index\" button).");
        return;
    }

    if (text.trimmed().isEmpty())
    {
        resultsList->clear();
        return;
    }

    std::string query = text.trimmed().toStdString();
    int mode = searchModeCombo->currentIndex(); // 0 Smart, 1 Exact, 2 Prefix, 3 Extension

    std::vector<std::string> results;

    if (mode == 3)
    {
        // Extension search: done at the GUI layer directly against the
        // indexed metadata, since the Trie only indexes file names.
        QString ext = text.trimmed();
        if (!ext.startsWith('.')) ext.prepend('.');
        ext = ext.toLower();

        for (const FileInfo &file : indexedFiles)
        {
            QString fileExt = QString::fromStdString(file.extension).toLower();
            if (fileExt == ext)
            {
                results.push_back(file.path);
            }
        }
        std::sort(results.begin(), results.end());
    }
    else if (mode == 1)
    {
        results = engine->exactSearch(query);
    }
    else if (mode == 2)
    {
        results = engine->prefixSearch(query);
    }
    else
    {
        results = engine->smartSearch(query);
    }

    if (results.empty())
    {
        showEmptyState("No files found.");
    }
    else
    {
        populateResults(results);
    }
}

void MainWindow::populateResults(const std::vector<std::string> &paths)
{
    resultsList->clear();
    for (const std::string &p : paths)
    {
        QString qpath = QString::fromStdString(p);
        FileInfo info = pathToInfo.value(qpath);

        QString label = QString("%1   (%2, %3)")
                             .arg(QString::fromStdString(info.name))
                             .arg(formatSize(info.size))
                             .arg(qpath);

        QListWidgetItem *item = new QListWidgetItem(label);
        item->setData(Qt::UserRole, qpath);
        resultsList->addItem(item);
    }
}

void MainWindow::showEmptyState(const QString &message)
{
    resultsList->clear();
    QListWidgetItem *item = new QListWidgetItem(message);
    item->setFlags(Qt::NoItemFlags);
    resultsList->addItem(item);
}

void MainWindow::onResultDoubleClicked(QListWidgetItem *item)
{
    QString path = item->data(Qt::UserRole).toString();
    if (path.isEmpty()) return;

    // Open the containing folder with the file selected (Windows Explorer).
    // QFileInfo fi(path);
    // QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absolutePath()));
    QDesktopServices::openUrl(
        QUrl::fromLocalFile(path));
}

// ---------------------------------------------------------------------
// Duplicate detection
// ---------------------------------------------------------------------
void MainWindow::findDuplicates()
{
    if (indexedFiles.empty())
    {
        QMessageBox::information(this, "No Index", "Index a folder first.");
        return;
    }

    duplicatesTree->clear();

    std::vector<std::vector<FileInfo>> groups =
        detector.findDuplicates(indexedFiles);

    if (groups.empty())
    {
        QTreeWidgetItem *none = new QTreeWidgetItem(duplicatesTree);
        none->setText(0, "No duplicate files found (matched by size).");
        return;
    }

    int groupNum = 1;
    for (const auto &group : groups)
    {
        QTreeWidgetItem *groupItem = new QTreeWidgetItem(duplicatesTree);
        groupItem->setText(0, QString("Group %1 (%2 files)").arg(groupNum++).arg(group.size()));
        groupItem->setExpanded(true);

        for (const FileInfo &file : group)
        {
            QTreeWidgetItem *child = new QTreeWidgetItem(groupItem);
            child->setText(0, QString::fromStdString(file.name));
            child->setText(1, formatSize(file.size));
            child->setText(2, QString::fromStdString(file.path));
            child->setData(0, Qt::UserRole, QString::fromStdString(file.path));
        }
    }

    statusLabel->setText(QString("Found %1 duplicate group(s).").arg(groups.size()));
}

void MainWindow::onDuplicateDoubleClicked(QTreeWidgetItem *item, int)
{
    QString path = item->data(0, Qt::UserRole).toString();
    if (path.isEmpty()) return;

    // QFileInfo fi(path);
    // QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absolutePath()));
    QDesktopServices::openUrl(
        QUrl::fromLocalFile(path));
}

// ---------------------------------------------------------------------
// Export
// ---------------------------------------------------------------------
void MainWindow::exportResults()
{
    if (resultsList->count() == 0)
    {
        QMessageBox::information(this, "Nothing to Export", "Run a search first.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Export Results",
                                                      "search_results.txt", "Text Files (*.txt)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Export Failed", "Could not write to that file.");
        return;
    }

    QTextStream out(&file);
    for (int i = 0; i < resultsList->count(); ++i)
    {
        out << resultsList->item(i)->text() << "\n";
    }
    file.close();

    statusLabel->setText("Results exported to " + fileName);
}

// ---------------------------------------------------------------------
// Dark mode
// ---------------------------------------------------------------------
void MainWindow::toggleDarkMode(bool enabled)
{
    applyDarkPalette(enabled);
}

void MainWindow::applyDarkPalette(bool dark)
{
    if (!dark)
    {
        qApp->setPalette(QApplication::style()->standardPalette());
        return;
    }

    QPalette p;
    p.setColor(QPalette::Window, QColor(37, 37, 38));
    p.setColor(QPalette::WindowText, Qt::white);
    p.setColor(QPalette::Base, QColor(30, 30, 30));
    p.setColor(QPalette::AlternateBase, QColor(45, 45, 45));
    p.setColor(QPalette::Text, Qt::white);
    p.setColor(QPalette::Button, QColor(53, 53, 53));
    p.setColor(QPalette::ButtonText, Qt::white);
    p.setColor(QPalette::Highlight, QColor(0, 122, 204));
    p.setColor(QPalette::HighlightedText, Qt::white);
    qApp->setPalette(p);
}

// ---------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------
QString MainWindow::formatSize(uintmax_t bytes) const
{
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    double size = static_cast<double>(bytes);
    int unit = 0;
    while (size >= 1024.0 && unit < 4)
    {
        size /= 1024.0;
        unit++;
    }
    return QString::number(size, 'f', unit == 0 ? 0 : 1) + " " + units[unit];
}

QString MainWindow::elapsedString(qint64 ms) const
{
    if (ms < 1000) return QString("%1 ms").arg(ms);
    return QString::number(ms / 1000.0, 'f', 2) + " s";
}
