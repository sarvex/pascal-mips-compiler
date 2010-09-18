#include "utils.h"

std::string Utils::toLower(std::string s)
{
    for(unsigned int i=0; i<s.size(); ++i) {
        s[i] = std::tolower(s[i]);
    }
    return s;
}

std::string Utils::toUpper(std::string s)
{
    for(unsigned int i=0; i<s.size(); ++i) {
        s[i] = std::toupper(s[i]);
    }
    return s;
}
