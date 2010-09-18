#include "utils.h"

std::string Utils::to_lower(std::string s)
{
    for(unsigned int i=0; i<s.size(); ++i) {
        s[i] = std::tolower(s[i]);
    }
    return s;
}

std::string Utils::to_upper(std::string s)
{
    for(unsigned int i=0; i<s.size(); ++i) {
        s[i] = std::toupper(s[i]);
    }
    return s;
}

