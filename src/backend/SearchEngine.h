#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include<iostream>
#include<vector>
#include<string>

#include "Ranking.h"
#include "Trie.h"

using namespace std;


class SearchEngine
{

private:

    Trie* searchTree;
    Ranking ranker;
    // vector<FileInfo> indexedFiles;

public:

    SearchEngine(Trie* tree);
    string toLowerCase(string word);

    vector<string> tokenize(string word);

    vector<string> exactSearch(string fileName);

    vector<string> prefixSearch(string prefix);

    // vector<string> extensionSearch(string extension);
    
    vector<string> smartSearch(string query);


};


#endif