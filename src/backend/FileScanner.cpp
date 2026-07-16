#include "FileScanner.h"
#include <filesystem>

namespace fs = std::filesystem;

vector<string> FileScanner::scanFiles(string path, bool recursive)
{
    vector<string> files;

    if (recursive)
    {
        for (auto& entry : fs::recursive_directory_iterator(
                 path, fs::directory_options::skip_permission_denied))
        {
            // Only keep actual files - directories would crash
            // FileIndexer when it tries to read a file size.
            if (entry.is_regular_file())
            {
                files.push_back(entry.path().string());
            }
        }
    }
    else
    {
        for (auto& entry : fs::directory_iterator(path))
        {
            if (entry.is_regular_file())
            {
                files.push_back(entry.path().string());
            }
        }
    }

    return files;
}
