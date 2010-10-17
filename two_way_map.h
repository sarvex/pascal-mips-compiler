#ifndef TWO_WAY_MAP_H
#define TWO_WAY_MAP_H

#include <map>
#include <set>

template <class K, class V>
class TwoWayMap
{
public:
    void associate(K key, V value);
    std::set<K> * keys(V value);
    V get(K key);
    // make this TwoWayMap equal to other
    void associate_all(const TwoWayMap & other);

    bool is_empty() const;
private:
    std::map<K, V> forward;
    std::map<V, std::set<K> *> backward;
};

template <class K, class V>
void TwoWayMap<K, V>::associate_all(const TwoWayMap & other) {
    for (typename std::map<K, V>::const_iterator it = other.forward.begin(); it != other.forward.end(); ++it) {
        associate(it->first, it->second);
    }
}

template <class K, class V>
bool TwoWayMap<K, V>::is_empty() const {
    return forward.size() == 0;
}

template <class K, class V>
std::set<K> * TwoWayMap<K, V>::keys(V value){
    if (backward.count(value))
        return backward.at(value);
    else
        return NULL;
}

template <class K, class V>
V TwoWayMap<K, V>::get(K key){
    return forward.at(key);
}

template <class K, class V>
void TwoWayMap<K, V>::associate(K key, V value){
    // destroy old association
    if (forward.count(key)) {
        V old_value = forward[key];
        std::set<K> * set = backward[old_value];
        set->erase(key);
        if (set->size() == 0) {
            delete set;
            backward.erase(old_value);
        }
    }

    // make new one
    forward[key] = value;
    std::set<K> * set;
    if (backward.count(value)) {
        set = backward[value];
    } else {
        set = new std::set<K>();
        backward[value] = set;
    }
    set->insert(key);

}

#endif // TWO_WAY_MAP_H
