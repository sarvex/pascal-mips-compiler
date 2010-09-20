#ifndef INSENSITIVE_MAP_H
#define INSENSITIVE_MAP_H

#include <map>
#include <vector>

#include "utils.h"

template <class T>
class OrderedInsensitiveMap {
    private:
        std::map<std::string, T> m_map;
        std::map<std::string, std::string> m_original_case;
        std::vector<T> m_vector;
    public:
        OrderedInsensitiveMap() : m_map(), m_original_case() {}

        int count() { return m_vector.size(); }
        bool has_key(std::string x) { return m_map.count(Utils::to_lower(x)) > 0; }
        T item(std::string x) { return m_map[Utils::to_lower(x)]; }
        T item(int i) { return m_vector[i]; }
        void put(std::string key, T value);
        std::string original_case(std::string x) { return m_original_case[Utils::to_lower(x)]; }
};

template <class T>
void OrderedInsensitiveMap<T>::put(std::string key, T value) {
    std::string lower_key = Utils::to_lower(key);
    m_map[lower_key] = value;
    m_original_case[lower_key] = key;
    m_vector.push_back(value);
}

#endif
