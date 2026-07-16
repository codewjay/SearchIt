#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QListWidget>
#include <QTreeWidget>
#include <QTabWidget>
#include <QLabel>
#include <QMap>
#include <vector>
#include <string>

#include "../backend/FileInfo.h"
#include "../backend/FileScanner.h"
#include "../backend/FileIndexer.h"
#include "../backend/Trie.h"
#include "../backend/SearchEngine.h"
#include "../backend/DuplicateDetector.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void browseDirectory();
    void scanAndIndex();
    void performSearch(const QString &text);
    void searchModeChanged(int index);
    void findDuplicates();
    void onResultDoubleClicked(QListWidgetItem *item);
    void onDuplicateDoubleClicked(QTreeWidgetItem *item, int column);
    void toggleDarkMode(bool enabled);
    void exportResults();

private:
    void setupUi();
    void applyDarkPalette(bool dark);
    void populateResults(const std::vector<std::string> &paths);
    void showEmptyState(const QString &message);
    QString formatSize(uintmax_t bytes) const;
    QString elapsedString(qint64 ms) const;

    // --- Top bar (indexing) ---
    QLineEdit   *dirPathEdit;
    QPushButton *browseButton;
    QPushButton *scanButton;
    QCheckBox   *recursiveCheck;
    QCheckBox   *darkModeCheck;
    QLabel      *statusLabel;

    // --- Search tab ---
    QLineEdit   *searchEdit;
    QComboBox   *searchModeCombo;
    QListWidget *resultsList;
    QPushButton *exportButton;

    // --- Duplicates tab ---
    QPushButton *findDuplicatesButton;
    QTreeWidget *duplicatesTree;

    QTabWidget  *tabs;

    // --- Backend ---
    FileScanner scanner;
    FileIndexer indexer;
    Trie searchTree;
    SearchEngine *engine;
    DuplicateDetector detector;

    std::vector<FileInfo> indexedFiles;
    QMap<QString, FileInfo> pathToInfo; // path -> metadata, for quick lookup
};

#endif // MAINWINDOW_H
