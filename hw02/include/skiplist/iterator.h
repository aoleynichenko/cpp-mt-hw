#ifndef __ITERATOR_H
#define __ITERATOR_H
#include "node.h"
#include <cassert>

/**
 * Skiplist const iterator
 */
template <class Key, class Value>
class Iterator {
private:
    Node<Key, Value>* pCurrent;

public:
    Iterator(Node<Key, Value>* p)
        : pCurrent(p) {}
    virtual ~Iterator() {}

    virtual const Key& key() const {
        assert(pCurrent != nullptr);
        return pCurrent->key();
    };

    virtual const Value& value() const {
        assert(pCurrent != nullptr);
        return pCurrent->value();
    };

    virtual const Value& operator*() {
        assert(pCurrent != nullptr);
        return pCurrent->value();
    };

    virtual const Value& operator->() {
        assert(pCurrent != nullptr);
        return pCurrent->value();
    };

    virtual bool operator==(const Iterator& other) const {
        return pCurrent == other.pCurrent;
    };

    virtual bool operator!=(const Iterator& other) const {
        return !(*this == other);
    };

    virtual Iterator& operator=(const Iterator& other) {
        pCurrent = other.pCurrent;
        return *this;
    };

    virtual Iterator& operator++() {
        pCurrent = &pCurrent->next();
        return *this;
    };

    virtual Iterator& operator++(int) {
        pCurrent = &pCurrent->next();
        return *this;
    };
};

#endif // __ITERATOR_H
