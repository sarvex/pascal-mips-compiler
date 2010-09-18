#ifndef _UTILS_H_
#define _UTILS_H_

#include <iostream>
#include <sstream>
#include <map>
#include <vector>

namespace Utils
{
    // return the lesser of the two args
    template <class T>
    T min(T x, T y) { return (x > y) ? y : x; }

    // return the greater of two args
    template <class T>
    T max(T x, T y) { return (x > y) ? x : y; }

    // type conversion
    template <class T>
    T string_to(std::string str);

    template <class T>
    std::string to_string(T value);

    // string parsing
    std::string toLower(std::string s);
    std::string toUpper(std::string s);
}

template <class T>
T Utils::string_to(std::string str) {
    std::stringstream ss;
    ss << str;
    T out;
    ss >> out;
    return out;
}
template <class T>
std::string Utils::to_string(T value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}


#endif
