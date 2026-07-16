#ifndef DUPLICATEDETECTOR_H
#define DUPLICATEDETECTOR_H

#include<iostream>
#include<vector>
#include<unordered_map>

#include "FileInfo.h"

using namespace std;


class DuplicateDetector
{

public:

    vector<vector<FileInfo>>

    findDuplicates(

        vector<FileInfo> files

        );

};


#endif