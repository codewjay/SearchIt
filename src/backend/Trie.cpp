#include "Trie.h"



Trie::Trie()
{

    root = new TrieNode();

}

void Trie::insert(string word,string filePath)
{
    TrieNode* current = root;
    for(char letter : word)
    {
        if(current->children.find(letter)
        == current->children.end())
        {
            current->children[letter]
            = new TrieNode();
        }
        current =
        current->children[letter];
    }
    current->isEndOfWord = true;
    current->filePaths.push_back(
    filePath
    );


}

vector<string> Trie::search(string word)
{
    TrieNode* current = root;
    for(char letter : word)
    {
        if(current->children.find(letter)
            == current->children.end())
        {
            return {};
        }
        current =
        current->children[letter];
    }
    if(current->isEndOfWord == false)
    {
        return {};
    }
    return current->filePaths;
}


void Trie::collectResults(
    TrieNode* current,
    vector<string>& result)
{


    if(current->isEndOfWord)
    {
        for(string file :
            current->filePaths)
        {
            result.push_back(file);
        }
    }

    for(auto child :
        current->children)
    {
        collectResults(
        child.second,
        result
        );
    }


}

vector<string>Trie::prefixSearch(
        string prefix)
{
    TrieNode* current = root;
    for(char letter :
        prefix)
    {
        if(current->children.find(letter)
        == current->children.end())
        {
            return {};
        }
        current =
        current->children[letter];
    }
    vector<string> result;
    collectResults(
            current,
            result);
    return result;
}