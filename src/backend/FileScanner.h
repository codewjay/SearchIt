#ifndef FILESCANNER_H
#define FILESCANNER_H

#include<iostream>
#include<vector>
#include<string>

using namespace std;

class FileScanner
{

public:

    // recursive = false keeps the original top-level-only behavior.
    // Pass true to scan every subfolder as well.
    vector<string> scanFiles(string path, bool recursive = false);

};

#endif
