#ifndef TRIE_H
#define TRIE_H

#include<iostream>
#include<vector>
#include<string>
#include<map>

using namespace std;


class TrieNode
{

public:

    map<char,TrieNode*> children;

    bool isEndOfWord = false;

    vector<string> filePaths;

};



class Trie
{

private:

    TrieNode* root;


public:

    Trie();


    void insert(string word,
                string filePath);


    vector<string> search(
                string word);


    vector<string> prefixSearch(string prefix);
                
                
    void collectResults(TrieNode* current,vector<string>& result);
};

#endif