#include "SearchEngine.h"
#include<cctype>
#include<sstream>

SearchEngine::SearchEngine(Trie* tree)
{
    searchTree = tree;
    // indexedFiles = files;
}

string SearchEngine::toLowerCase(string word)
{
    for(char &letter : word)
    {
        letter = tolower(letter);
    }
    return word;

}



vector<string>SearchEngine::exactSearch(string fileName)
{
    fileName = toLowerCase(fileName);
    return searchTree->search(fileName);
}

vector<string>SearchEngine::prefixSearch(string prefix)
{
    prefix = toLowerCase(prefix);
    return searchTree->prefixSearch(prefix);
}

// vector<string>SearchEngine::extensionSearch(string extension)
// {
//     vector<string> results;
//     extension =toLowerCase(extension);
//     for(FileInfo file : indexedFiles)
//     {
//         string currentExtension = file.extension;
//         currentExtension =toLowerCase(currentExtension);
//         if(currentExtension == extension)
//         {
//             results.push_back(
//                 file.path);
//         }
//     }
//     return results;

// }

vector<string>SearchEngine::smartSearch(string query)
{
    query =toLowerCase(query);
    vector<string> result;
    // Try Exact Search first
    result = exactSearch(query);
    // result = extensionSearch(query);
    // Exact Search succeeded
    if(!result.empty())
    {
        return ranker.rankResults(result);
    }
    // Try Prefix Search
    result =prefixSearch(query);
    return ranker.rankResults(result);
}

vector<string>SearchEngine::tokenize(string word)
{
    vector<string> tokens;
    string token;
    stringstream stream(word);
    while(stream >> token)
    {
        tokens.push_back(token);
    }
    return tokens;
}