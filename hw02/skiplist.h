#ifndef SKIPLIST
#define SKIPLIST

//#include <utility>
#include <string.h>

template<typename K, typename V, int NLEV = 32>
class SkipList {
    struct Node {
        K key;
        V value;
        std::pair<K,V> p;
        Node* prev;
        Node* next[NLEV];
        
        Node(const K& key, const V& val);
    };
public:

    SkipList();
    ~SkipList();
    
    void insert(const K& key, const V& val);
    
    class Iterator {
        Node* curr;
    public:
        //val& operator*();  // much better is to return std::pair
    };

private:
    
    Node* head[NLEV];
};

template<typename K, typename V, int NLEV>
SkipList<K, V, NLEV>::SkipList()
{
    memset(head, 0, sizeof(Node*) * NLEV); // set head pointers to nullptr
}

template<typename K, typename V, int NLEV>
SkipList<K, V, NLEV>::~SkipList()
{
}

template<typename K, typename V, int NLEV>
SkipList<K, V, NLEV>::Node::Node(const K& key_, const V& val_)
    :   key(key_), value(val_), prev(nullptr)
{
    memset(next, 0, sizeof(Node*) * NLEV);
}

template<typename K, typename V, int NLEV>
void SkipList<K, V, NLEV>::insert(const K& key, const V& val)
{
    Node* node = new Node(key, val);
    if (*head == nullptr) { // list is empty now
        for (size_t i = 0; i < NLEV; i++)
            head[i] = node;
    }
}

#endif // SKIPLIST
