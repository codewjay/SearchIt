#include "DuplicateDetector.h"



vector<vector<FileInfo>>

DuplicateDetector::

findDuplicates(

        vector<FileInfo> files)
{

    vector<vector<FileInfo>>
    duplicates;


    unordered_map
    <
        unsigned long long,

        vector<FileInfo>

    > sizeMap;



    // Group all files according
    // to their size.

    for(FileInfo file : files)
    {
        sizeMap[file.size]
        .push_back(file);
    }



    // If more than one file has
    // the same size, they are
    // potential duplicates.

    for(auto group : sizeMap)
    {
        if(group.second.size() > 1)
        {
            duplicates.push_back(
                    group.second);
        }
    }



    return duplicates;

}