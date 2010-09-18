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

std::string Utils::err_header(int line_number)
{
    std::stringstream ss;
    ss << "ERROR: line " << line_number << ": ";
    return ss.str();
}
