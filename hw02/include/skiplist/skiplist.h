#ifndef __SKIPLIST_H
#define __SKIPLIST_H
#include "iterator.h"
#include "node.h"
#include <cstdlib>
#include <functional>
#include <stdio.h>

/**
 * Skiplist interface
 */
template <class Key, class Value, size_t MAXHEIGHT, class Compare = std::less<Key> >
class SkipList {
private:
    DataNode<Key, Value>* pHead;
    DataNode<Key, Value>* pTail;

    IndexNode<Key, Value>* pTailIdx;
    IndexNode<Key, Value>* aHeadIdx[MAXHEIGHT];

public:
    /**
   * Creates new empty skiplist
   */
    SkipList() {
        pHead = new DataNode<Key, Value>(nullptr, nullptr);
        pTail = new DataNode<Key, Value>(nullptr, nullptr);
        pHead->next(pTail);

        Node<Key, Value>* prev = pHead;
        pTailIdx = new IndexNode<Key, Value>(pTail, pTail);
        for (int i = 0; i < MAXHEIGHT; i++) {
            aHeadIdx[i] = new IndexNode<Key, Value>(prev, pHead);
            aHeadIdx[i]->next(pTailIdx);
            prev = aHeadIdx[i];
        }
    }

    /**
   * Disable copy constructor
   */
    SkipList(const SkipList& that) = delete;

    /**
   * Destructor
   */
    virtual ~SkipList() {
        delete pTailIdx;
        for (int i = 0; i < MAXHEIGHT; i++) {
            delete aHeadIdx[i];
        }

        delete pHead;
        delete pTail;
    }

    /**
   * Assign new value for the key. If a such key already has
   * assosiation then old value returns, otherwise nullptr
   *
   * @param key key to be assigned with value
   * @param value to be added
   * @return old value for the given key or nullptr
   */
    virtual Value* Put(const Key& key, const Value& value) const {
        return insert_pair(key, value);
    };

    /**
   * Put value only if there is no assosiation with key in
   * the list and returns nullptr
   *
   * If there is an established assosiation with the key
   * method doesn't nothing and returns existing value
   *
   * @param key key to be assigned with value
   * @param value to be added
   * @return existing value for the given key or nullptr
   */
    virtual Value* PutIfAbsent(const Key& key, const Value& value) {
        return insert_pair(key, value, false);
    };

    /**
   * Returns value assigned for the given key or nullptr
   * if there is no established assosiation with the given key
   *
   * @param key to find
   * @return value assosiated with given key or nullptr
   */
    virtual Value* Get(const Key& key) const {
        int lvl = MAXHEIGHT - 1;

        IndexNode<Key, Value>* ix = aHeadIdx[MAXHEIGHT - 1];
        while (lvl >= 0) {
            IndexNode<Key, Value>* nxt = dynamic_cast<IndexNode<Key, Value>*>(&ix->next());

            if (nxt == pTailIdx || cmp_less(key, nxt->key())) {
                lvl--;
                ix = dynamic_cast<IndexNode<Key, Value>*>(ix->down());
            } else if (cmp_less(nxt->key(), key)) {
                ix = nxt;
            } else {
                return &nxt->value();
            }
        }
        return nullptr;
    };

    /**
   * Remove given key from the skpiplist and returns value
   * it has or nullptr in case if key wasn't assosiated with
   * any value
   *
   * @param key to be added
   * @return value for the removed key or nullptr
   */
    virtual Value* Delete(const Key& key) {
        int lvl = MAXHEIGHT - 1;

        IndexNode<Key, Value>* ix = aHeadIdx[MAXHEIGHT - 1];
        while (lvl >= 0) {
            IndexNode<Key, Value>* nxt = dynamic_cast<IndexNode<Key, Value>*>(&ix->next());

            if (nxt == pTailIdx || cmp_less(key, nxt->key())) {
                lvl--;
                ix = dynamic_cast<IndexNode<Key, Value>*>(ix->down());
            } else if (cmp_less(nxt->key(), key)) {
                ix = nxt;
            } else {
                // remove
                Value* ret_val = &nxt->value();
                DataNode<Key, Value> *dix, *dnxt;
                while (lvl >= 0) {
                    ix->next(dynamic_cast<IndexNode<Key, Value>*>(&nxt->next()));
                    if (lvl > 0) {
                        ix = dynamic_cast<IndexNode<Key, Value>*>(ix->down());
                        nxt = dynamic_cast<IndexNode<Key, Value>*>(nxt->down());
                    } else {
                        dix = dynamic_cast<DataNode<Key, Value>*>(ix->down());
                        dnxt = dynamic_cast<DataNode<Key, Value>*>(nxt->down());
                    }
                    lvl--;
                }
                dix->next(dynamic_cast<DataNode<Key, Value>*>(&dnxt->next()));

                return ret_val;
            }
        }
        // nothing to remove
        return nullptr;
    };

    /**
   * Same as Get
   */
    virtual Value* operator[](const Key& key) {
        return Get(key);
    };

    /**
   * Return iterator onto very first key in the skiplist
   */
    virtual Iterator<Key, Value> cbegin() const {
        return Iterator<Key, Value>(&pHead->next());
    };

    /**
   * Returns iterator to the first key that is greater or equals to
   * the given key
   */
    virtual Iterator<Key, Value> cfind(const Key& min) const {
        int lvl = MAXHEIGHT - 1;

        IndexNode<Key, Value>* ix = aHeadIdx[MAXHEIGHT - 1];
        while (lvl >= 0) {
            IndexNode<Key, Value>* nxt = dynamic_cast<IndexNode<Key, Value>*>(&ix->next());

            if (nxt == pTailIdx || cmp_less(min, nxt->key())) {
                lvl--;
                ix = dynamic_cast<IndexNode<Key, Value>*>(ix->down());
            } else if (cmp_less(nxt->key(), min)) {
                ix = nxt;
            } else {
                return Iterator<Key, Value>(nxt->root());
            }
        }
        return Iterator<Key, Value>(pTail);
    };

    /**
   * Returns iterator on the skiplist tail
   */
    virtual Iterator<Key, Value> cend() const {
        return Iterator<Key, Value>(pTail);
    };

private:
    // comparator
    Compare cmp_less;

    Value* insert_pair(const Key& key, const Value& value, bool substitute = true) const {
        int lvl = MAXHEIGHT - 1;
        IndexNode<Key, Value>* trace[MAXHEIGHT];
        IndexNode<Key, Value>* ix = aHeadIdx[MAXHEIGHT - 1];

        while (ix != pTailIdx) {
            IndexNode<Key, Value>* nxt = dynamic_cast<IndexNode<Key, Value>*>(&ix->next());

            if (nxt == pTailIdx || cmp_less(key, nxt->key())) { // descent
                trace[lvl] = ix;
                lvl--;
                ix = dynamic_cast<IndexNode<Key, Value>*>(ix->down());
            } else if (cmp_less(nxt->key(), key)) {
                ix = nxt;
                continue;
            } else { // (key == ix->key()) { // точное совпадение ключей -> заменяем Value
                if (substitute) { // usual put
                    Value* v = new Value(value);
                    return nxt->value(v);
                } else { // put if absent
                    return &nxt->value();
                }
            }
            if (lvl == -1) {
                DataNode<Key, Value>* curr_data = dynamic_cast<DataNode<Key, Value>*>(trace[0]->down());
                DataNode<Key, Value>* data = new DataNode<Key, Value>(new Key(key), new Value(value));
                Node<Key, Value>* down = data;

                data->next(dynamic_cast<DataNode<Key, Value>*>(&curr_data->next()));
                curr_data->next(data);
                for (int i = 0; i < MAXHEIGHT; i++) {
                    if (i != 0 && (rand() % 2 == 0)) {
                        break;
                    }
                    IndexNode<Key, Value>* prev = trace[i];
                    IndexNode<Key, Value>* node = new IndexNode<Key, Value>(down, data);
                    node->next(dynamic_cast<IndexNode<Key, Value>*>(&prev->next()));
                    prev->next(node);
                    down = node;
                }
                return nullptr;
            }
        }
    }
};
#endif // __SKIPLIST_H
