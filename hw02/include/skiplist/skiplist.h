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
        //printf("Get %d\n", key);

        IndexNode<Key, Value>* ix = aHeadIdx[MAXHEIGHT - 1];
        //ix = dynamic_cast<IndexNode<Key, Value>*>(&ix->next());
        while(lvl >= 0) {
            //printf("loop!\n");
            IndexNode<Key, Value>* nxt = dynamic_cast<IndexNode<Key, Value>*>(&ix->next());
            //printf("nxt == pTailIdx: %s\n", (nxt == pTailIdx) ? "true" : "false");
            //if (nxt != pTailIdx)
            //    printf("nxt->key == %d\n", nxt->key());
            if (nxt == pTailIdx || cmp_less(key, nxt->key())) {
                lvl--;
                ix = dynamic_cast<IndexNode<Key, Value>*>(ix->down());
                //printf("move down to lvl %d\n", lvl);
                continue;
            } else if (cmp_less(nxt->key(), key)) {
                ix = nxt;
                //printf("move right");
                continue;
            } else {
                //printf("found!");
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

        for (IndexNode<Key, Value>* ix = aHeadIdx[MAXHEIGHT - 1]; ix != pTailIdx;) {
            IndexNode<Key, Value>* nxt = dynamic_cast<IndexNode<Key, Value>*>(&ix->next());
            //printf("ix = %p   nxt = %p\n", ix, nxt);
            // достигли конца списка, вставляем элемент сюда
            // то есть ПОСЛЕ текущего элемента списка
            if (nxt == pTailIdx || cmp_less(key, nxt->key())) {
                // вставить сюда если lvl == 0
                // иначе спуститься пониже
                //printf("goto level %d -> ", lvl);
                //printf("%d\n", lvl);
                trace[lvl] = ix;
                // descent
                lvl--;
                ix = dynamic_cast<IndexNode<Key, Value>*>(ix->down());
                // fill trace
                //continue;
            } else if (cmp_less(nxt->key(), key)) {
                // идем дальше по списку
                ix = nxt;
                continue;
            } else { // (key == ix->key()) { // точное совпадение ключей -> заменяем Value
                if (substitute) { // usual put
                    Value* v = new Value(value);
                    return ix->value(v);
                } else { // put if absent
                    return &ix->value();
                }
            }
            if (lvl == 0) {
                trace[0] = ix;
                /*printf("level = 0 -> do insert\n");
            printf("pTailIdx = %p\n", pTailIdx);
            printf("trace[] = \n");
            for (int i = 0; i < MAXHEIGHT; i++)
              printf("%2d %p -> %p\n", i, trace[i], &trace[i]->next());*/
                // вставляем элемент
                DataNode<Key, Value>* curr_data = dynamic_cast<DataNode<Key, Value>*>(ix->down());
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
