#ifndef __SKIPLIST_H
#define __SKIPLIST_H
#include "iterator.h"
#include "node.h"
#include <functional>
#include <stdio.h>

/**
 * Skiplist interface
 */
template <class Key, class Value, size_t MAXHEIGHT, class Compare = std::less<Key> >
class SkipList {
private:
    Node<Key, Value>* pHead;
    Node<Key, Value>* pTail;

    IndexNode<Key, Value>* pTailIdx;
    IndexNode<Key, Value>* aHeadIdx[MAXHEIGHT];

public:
    /**
   * Creates new empty skiplist
   */
    SkipList() {
        pHead = new DataNode<Key, Value>(nullptr, nullptr);
        pTail = new DataNode<Key, Value>(nullptr, nullptr);

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
        int level = MAXHEIGHT-1;
        IndexNode<Key,Value>* ix = aHeadIdx[level];
        for (; ix != pTailIdx; ix = (IndexNode<Key,Value>*) &ix->next()) {
            IndexNode<Key,Value>* nxt = (IndexNode<Key,Value>*) &ix->next();
            const Key& nxt_key = nxt->key();
            if (nxt == pTailIdx) {
                // добежали до конца, вставляем пирамидку
            }
            else if (nxt_key > key) {
                level--;
                // ix = ix->down()
                // шаг вниз.
                // если шагать уже некуда,
                // вставить пирамидку прямо сюда, перед ix
            }
            else if (nxt_key == key) {  // ключ найден, обновим значение
                Value& old = const_cast<Value&>(ix->value());
                Value ret = old;
                old = value;
                return &old;
            }
            else  // просто продолжаем перебор элементов списка
              continue;

        }
        return nullptr;
    };

    /**
   * Put value only if there is no association with key in
   * the list and returns nullptr
   *
   * If there is an established association with the key already
   * method doesn't nothing and returns existing value
   *
   * @param key key to be assigned with value
   * @param value to be added
   * @return existing value for the given key or nullptr
   */
    virtual Value* PutIfAbsent(const Key& key, const Value& value) {
        return nullptr;
    };

    /**
   * Returns value assigned for the given key or nullptr
   * if there is no established assosiation with the given key
   *
   * @param key to find
   * @return value assosiated with given key or nullptr
   */
    virtual Value* Get(const Key& key) const {
        return nullptr;
    };

    /**
   * Removes given key from the skiplist and returns value
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
    virtual const Value* operator[](const Key& key) {
        return nullptr;
    };

    /**
   * Return iterator onto very first key in the skiplist
   */
    virtual Iterator<Key, Value> cbegin() const {
        return Iterator<Key, Value>(pTail);
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
};
#endif // __SKIPLIST_H
