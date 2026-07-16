#ifndef FILEINDEXER_H
#define FILEINDEXER_H

#include<iostream>
#include<vector>
#include<string>

#include "FileInfo.h"

using namespace std;

class FileIndexer{
    public:

        // 'files' is a list of paths as returned by FileScanner
        // (already full/relative paths, not bare filenames).
        vector<FileInfo> indexFiles(vector<string> files, string directoryPath);
};

#endif
