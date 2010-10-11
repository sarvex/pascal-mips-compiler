#ifndef INSENSITIVE_MAP_H
#define INSENSITIVE_MAP_H

#include <map>
#include <vector>

#include "utils.h"

template <class T>
class OrderedInsensitiveMap {
    private:
        std::map<std::string, T> m_map;
        std::vector<T> m_vector;
    public:
        OrderedInsensitiveMap() : m_map() {}

        int count() { return m_vector.size(); }
        bool has_key(std::string x) { return m_map.count(Utils::to_lower(x)) > 0; }
        T get(std::string x) { return m_map[Utils::to_lower(x)]; }
        T get(int i) { return m_vector[i]; }
        void put(std::string key, T value);
};

template <class T>
void OrderedInsensitiveMap<T>::put(std::string key, T value) {
    m_map[Utils::to_lower(key)] = value;
    m_vector.push_back(value);
}

#endif
