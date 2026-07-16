#include "Ranking.h"


vector<string>Ranking::rankResults(vector<string> results)
{
    sort(results.begin(),results.end());
    return results;
}