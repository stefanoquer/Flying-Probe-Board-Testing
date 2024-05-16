#include "Utils.h"
#include <sstream>

using namespace std;

void splitLine(const std::string &inLine, const char separator, std::vector<std::string> &results)
{
    results.clear();
    istringstream lineSS(inLine);
    string token;
    while(getline(lineSS, token, separator)) results.emplace_back(token);
}