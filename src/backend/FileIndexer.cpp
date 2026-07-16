#include "FileIndexer.h"
#include <filesystem>

namespace fs = std::filesystem;


vector<FileInfo> FileIndexer::indexFiles(vector<string> files, string directoryPath)
{
    vector<FileInfo> indexedFiles;

    for(string entry : files)
    {
        FileInfo file;

        // FileScanner already gives us a full path for every entry,
        // so we use it directly instead of re-joining with directoryPath
        // (the old code did fs::path(directoryPath) / fileName, which
        // silently produced the wrong 'name' field whenever fileName
        // was already a path rather than a bare filename).
        fs::path filePath = fs::path(entry);

        // Store just the file name (e.g. "notes.txt").
        file.name = filePath.filename().string();
        // Store the complete path.
        file.path = filePath.string();
        // Store the file size.
        file.size = fs::file_size(filePath);
        // Store the extension.
        file.extension = filePath.extension().string();
        // Store the last modified time.
        auto time = fs::last_write_time(filePath);
        file.lastModified = time.time_since_epoch().count();

        indexedFiles.push_back(file);
    }
    return indexedFiles;
}
