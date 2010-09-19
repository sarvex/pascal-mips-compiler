
#include <map>

#include "utils.h"

template <class T>
class InsensitiveMap {
    private:
        std::map<std::string, T> m_map;
        std::map<std::string, std::string> m_original_case;
    public:
        InsensitiveMap() : m_map(), m_original_case() {}
        int count(std::string & x) { return m_map.count(x); }
        T operator[] (std::string & x) { return m_map[x]; }
        void put(std::string key, T value);
};

template <class T>
void InsensitiveMap<T>::put(std::string key, T value) {
    m_map[key] = value;
    m_original_case[key] = key;
}
