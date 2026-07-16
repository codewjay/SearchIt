#ifndef FILEINFO_H
#define FILEINFO_H

#include<iostream>
#include<string>
#include<cstdint>

using namespace std;

struct FileInfo{
    string name;

    string path;

    uintmax_t size;

    string extension;

    long long lastModified;
};

#endif