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

        Node<Key, Value>* prev = nullptr;//pHead;
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
        // delete all index nodes
        for (int i = 0; i < MAXHEIGHT; i++) {
            for (Node<Key,Value>* ix = aHeadIdx[i]; ix != pTailIdx; ) {
                Node<Key,Value>* nxt = &ix->next();
                delete ix;
                ix = nxt;
            }
        }
        delete pTailIdx;

        for (Node<Key,Value>* p = &pHead->next(); p != pTail; ) {
            Node<Key,Value>* nxt = &p->next();
            delete &p->key();
            delete &p->value();
            delete p;
            p = nxt;
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
        DataNode<Key,Value>* node = find(key);
        return (node == nullptr) ? nullptr : &node->value();
    }

    /**
   * Remove given key from the skpiplist and returns value
   * it has or nullptr in case if key wasn't assosiated with
   * any value
   *
   * @param key to be added
   * @return value for the removed key or nullptr
   */
    virtual Value* Delete(const Key& key) {
        IndexNode<Key, Value>* ix = aHeadIdx[MAXHEIGHT - 1];

        while (ix != nullptr) {
            IndexNode<Key, Value>* nxt = dynamic_cast<IndexNode<Key, Value>*>(&ix->next());

            if (nxt == pTailIdx || cmp_less(key, nxt->key())) {
                ix = ix->down();
            } else if (cmp_less(nxt->key(), key)) {
                ix = nxt;
            } else {
                // delete stack of IndexNode's and 1 DataNode object
                Value* ret_val = &nxt->value(); // extract object to be returned

                DataNode<Key, Value>* data_curr;// = ix->root();
                DataNode<Key, Value>* data_del = nxt->root(); // DataNode to be deleted
                while (ix != nullptr) {
                    IndexNode<Key,Value>* nxt2 = dynamic_cast<IndexNode<Key, Value>*>(&nxt->next());

                    IndexNode<Key,Value>* idx_del = nxt; // IndexNode to be deleted
                    // remove IndexNode from linked list
                    while (&ix->next() != nxt)
                        ix = dynamic_cast<IndexNode<Key, Value>*>(&ix->next());
                    ix->next(nxt2);
                    data_curr = ix->root();
                    // descent
                    ix = ix->down();
                    nxt = nxt->down();
                    delete idx_del;
                }

                data_curr->next(dynamic_cast<DataNode<Key, Value>*>(&data_del->next()));
                // we should not delete value, because we return it!
                delete &data_del->key();
                delete data_del;

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
        DataNode<Key,Value>* node = find(min);
        return Iterator<Key, Value>((node == nullptr) ? pTail : node);
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

    /**
     * Returns pointer to DataNode with this key
     */
    DataNode<Key,Value>* find(const Key& key) const {
        int lvl = MAXHEIGHT - 1;

        IndexNode<Key, Value>* ix = aHeadIdx[MAXHEIGHT - 1];
        while (lvl >= 0) {
            IndexNode<Key, Value>* nxt = dynamic_cast<IndexNode<Key, Value>*>(&ix->next());

            if (nxt == pTailIdx || cmp_less(key, nxt->key())) {
                lvl--;
                ix = ix->down();
            } else if (cmp_less(nxt->key(), key)) {
                ix = nxt;
            } else {
                return nxt->root();
            }
        }
        return nullptr;
    };

    Value* insert_pair(const Key& key, const Value& value, bool substitute = true) const {
        int lvl = MAXHEIGHT - 1;
        IndexNode<Key, Value>* trace[MAXHEIGHT];
        IndexNode<Key, Value>* ix = aHeadIdx[MAXHEIGHT - 1];

        while (ix != pTailIdx) {
            IndexNode<Key, Value>* nxt = dynamic_cast<IndexNode<Key, Value>*>(&ix->next());

            if (nxt == pTailIdx || cmp_less(key, nxt->key())) { // descent
                trace[lvl] = ix;
                lvl--;
                ix = ix->down();
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
                /*DataNode<Key, Value>* curr_data = dynamic_cast<DataNode<Key, Value>*>(trace[0]->down());
                DataNode<Key, Value>* data = new DataNode<Key, Value>(new Key(key), new Value(value));
                Node<Key, Value>* down = data;*/
                DataNode<Key, Value>* curr_data = trace[0]->root();
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
