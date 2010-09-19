
#include <map>

#include "utils.h"

template <class T>
class InsensitiveMap {
    private:
        std::map<std::string, T> m_map;
        std::map<std::string, std::string> m_original_case;
    public:
        InsensitiveMap() : m_map(), m_original_case() {}

        bool has_key(std::string x) { return m_map.count(Utils::to_lower(x)) > 0; }
        T item(std::string x) { return m_map[Utils::to_lower(x)]; }
        void put(std::string key, T value);
        std::string original_case(std::string x) { return m_original_case[Utils::to_lower(x)]; }
};

template <class T>
void InsensitiveMap<T>::put(std::string key, T value) {
    std::string lower_key = Utils::to_lower(key);
    m_map[lower_key] = value;
    m_original_case[lower_key] = key;
}

