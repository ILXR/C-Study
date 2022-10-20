// #include <iostream>
// #include <list>
// #include <unordered_map>
// #include <memory>
// #include <mutex>
// #include <functional>

// using namespace std;

#include <iostream>
#include <list>
#include <unordered_map>
#include <memory>
#include <mutex>
using namespace std;
template <typename K, typename V>
class LRU_Cache
{
public:
    typedef pair<K, shared_ptr<V>> node;
    typedef typename list<node>::iterator iterator;
    LRU_Cache(int cap) : capacity(cap) {}
    shared_ptr<V> get(K key)
    {
        lock_guard<mutex> guard(mutex_t);
        auto ite = lru_map.find(key);
        if (ite == lru_map.end())
        {
            return shared_ptr<V>(nullptr);
        }
        lru_list.erase(ite->second);
        lru_list.emplace_front(*ite->second);
        lru_map[key] = lru_list.begin();
        return lru_map[key]->second;
    }
    void put(K key, shared_ptr<V> val)
    {
        lock_guard<mutex> guard(mutex_t);
        auto ite = lru_map.find(key);
        if (ite != lru_map.end())
        {
            lru_list.erase(ite->second);
        }
        lru_list.emplace_front(key, val);
        lru_map[key] = lru_list.begin();
        if (lru_list.size() > capacity)
        {
            lru_map.erase(lru_list.back().first);
            lru_list.pop_back();
        }
    }

private:
    int capacity;
    mutex mutex_t;
    list<node> lru_list;
    unordered_map<K, iterator> lru_map;
};

// template <typename K, typename V>
// class LRU_Cache
// {
// public:
//     typedef pair<K, shared_ptr<V>> node;
//     typedef typename list<node>::iterator iterator;
//     LRU_Cache(int cap) : capacity(cap) {}
//     void insert(K key, shared_ptr<V> val)
//     {
//         lock_guard<mutex> guard(mutex_t);
//         auto ite = lru_map.find(key);
//         if (ite != lru_map.end())
//         {
//             lru_list.erase(ite->second);
//         }
//         lru_list.emplace_front(key, val);
//         lru_map[key] = lru_list.begin();
//         if (lru_list.size() > capacity)
//         {
//             lru_map.erase(lru_list.back().first);
//             lru_list.pop_back();
//         }
//     }
//     shared_ptr<V> get(K key)
//     {
//         lock_guard<mutex> guard(mutex_t);
//         auto ite = lru_map.find(key);
//         return ite == lru_map.end() ? shared_ptr<V>() : ite->second->second;
//     }

// private:
//     int capacity;
//     mutex mutex_t;
//     list<node> lru_list;
//     unordered_map<K, iterator> lru_map;
// };

int main()
{
    LRU_Cache<int, int> cache(10);
    for (int i = 0; i < 15; i++)
    {
        cache.put(i, make_shared<int>(i));
        auto ptr = cache.get(i % 10);
        if (ptr)
            cout << *ptr << endl;
        else
            cout << "null" << endl;
    }
}