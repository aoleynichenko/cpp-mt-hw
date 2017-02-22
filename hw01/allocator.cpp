#include "allocator.h"
#include "allocator_error.h"
#include "allocator_pointer.h"

#include <stdio.h>
#include <string.h>

Allocator::Allocator(void* base, size_t size)
    : buf(base)
    , bufsize(size) {
    meta = (MetaInfo*)((char*)buf + bufsize);
    size_t sz = ((char*)meta - (char*)buf - sizeof(MetaInfo)) / sizeof(void*);
    MetaInfo* first_block = addMeta(buf, sz);
    first_block->setOccupied(false);
    insertFree(first_block);
}

Pointer Allocator::alloc(size_t N) {
    size_t nunits = (N + sizeof(void*) - 1) / sizeof(void*) + 1;

    for (MetaInfo* p = head.next; p != nullptr; p = p->next) {
        size_t sz = p->getSize();
        if (sz == nunits) {
            p->setOccupied(true); // free -> occupied
            MetaInfo* info = unlinkFree(p);
            return Pointer(indexOfInfo(info), this);
        } else if (sz >= nunits) {
            // we return block of sizeof(void*)*bytes
            // tail remains free, we update its metainfo
            void* mem = (char*)buf + p->offs;
            MetaInfo* occ_meta = addMeta(mem, nunits);
            occ_meta->setOccupied(true);

            void* mem_free = (char*)buf + p->offs + nunits * sizeof(void*);
            *((MetaInfo**)mem_free) = p;
            p->setSize(p->getSize() - nunits);
            p->offs = (char*)mem_free - (char*)buf;

            return Pointer(indexOfInfo(occ_meta), this);
        }
    }
    throw AllocError(AllocErrorType::NoMemory, "not enough memory");
}

void Allocator::realloc(Pointer& p, size_t N) {
    size_t nunits = (N + sizeof(void*) - 1) / sizeof(void*) + 1;

    void* from = p.get(); // will be nullptr if p == nullptr
    free(p);
    p = alloc(N);
    void* to = p.get();
    if (from) {
        memmove(to, from, (nunits - 1) * sizeof(void*));
    }
}

void Allocator::free(Pointer& p) {
    if (!p) {
        return;
    }

    MetaInfo* info = getMeta(p.idx_);
    info->setOccupied(false);
    insertFree(info);

    MetaInfo* last = getMeta(0);
    char *t, *start = (char *)buf + info->offs;
    char* lim = (char*)buf + last->offs + sizeof(void*) * last->getSize();
    // merge this free block with subsequent free blocks in memory
    for (t = start; t < lim;) {
        MetaInfo* hd = *((MetaInfo**)t);
        if (hd->isOccupied()) {
            break;
        }
        unlinkFree(hd);
        t += hd->getSize() * sizeof(void*);
    }
    info->setSize((t - start) / sizeof(void*));
    info->setOccupied(false);
    insertFree(info);
    p.reset();
}

void Allocator::defrag() {
    MetaInfo* last = getMeta(0); // last free block
    char* lim = (char*)buf + last->offs + sizeof(void*) * last->getSize();
    char *p, *to;

    for (to = p = (char*)buf; p < lim;) {
        MetaInfo* hd = *((MetaInfo**)p);
        size_t sz = hd->getSize();
        if (hd->isOccupied()) {
            memmove(to, p, sz * sizeof(void*));
            *((MetaInfo**)to) = hd;
            hd->offs = to - (char*)buf;
            to += sz * sizeof(void*);
        } else {
            unlinkFree(hd);
        }
        p += sz * sizeof(void*);
    }

    // merge all the remaining bytes into a single free block
    *((MetaInfo**)to) = last;
    last->offs = to - (char*)buf;
    last->setSize(((char*)meta - to) / sizeof(void*));
    last->setOccupied(false);
    head.next = last;
    last->prev = &head;
}

std::string Allocator::dump() const {
    return "";
}

// private methods

Allocator::MetaInfo* Allocator::addMeta(void* start, size_t nunits) {
    MetaInfo* last = getMeta(0);

    // try to reuse unused metainformation objects
    for (MetaInfo* mi = last-1; mi >= meta; mi--) {
        if (!mi->isUsed()) {
            mi->setSize(nunits);
            mi->offs = (char*)start - (char*)buf;
            *((MetaInfo**)start) = mi;
            return mi;
        }
    }

    // no MetaInfo objects found to be reused; create a new one
    char* lim = (char*)buf + last->offs;
    if (lim >= (char*)(meta - 1)) {
        throw AllocError(AllocErrorType::NoMemory, "not enough memory for metainfo");
    }

    meta--;
    // decrease size of last free block
    last->setSize(((char*)meta - ((char*)buf + last->offs)) / sizeof(void*));
    meta->setSize(nunits);
    meta->offs = (char*)start - (char*)buf;
    *((MetaInfo**)start) = meta;
    return meta;
}

// removes metainfo from double-linked list if free blocks
Allocator::MetaInfo* Allocator::unlinkFree(Allocator::MetaInfo* info) {
    if (info->prev) {
        info->prev->next = info->next;
        info->prev = nullptr;
    }
    if (info->next) {
        info->next->prev = info->prev;
        info->next = nullptr;
    }
    return info;
}

void* Allocator::get(size_t idx) {
    MetaInfo* end = (MetaInfo*)(buf + bufsize);
    MetaInfo* info = end - idx - 1;
    return (char*)buf + info->offs + sizeof(void*);
}

size_t Allocator::indexOfInfo(Allocator::MetaInfo* info) {
    MetaInfo* end = (MetaInfo*)((char*)buf + bufsize);
    size_t index = end - info - 1;
    return index;
}

Allocator::MetaInfo* Allocator::getMeta(size_t idx) {
    MetaInfo* end = (MetaInfo*)(buf + bufsize);
    MetaInfo* info = end - idx - 1;
    return info;
}

// insert metainfo to the beginning of double-linked list of free blocks
Allocator::MetaInfo* Allocator::insertFree(Allocator::MetaInfo* info) {
    if (head.next) {
        head.next->prev = info;
    }
    info->next = head.next;
    info->prev = &head;
    head.next = info;

    return info;
}

// class Allocator::MetaInfo

Allocator::MetaInfo::MetaInfo()
    : next(nullptr)
    , prev(nullptr)
    , size(0)
    , offs(0) {
}

const size_t Allocator::MetaInfo::sign_mask = 0x8000000000000000;

bool Allocator::MetaInfo::isOccupied() const {
    return (size & sign_mask) != 0;
}

bool Allocator::MetaInfo::isUsed() const {
    return isOccupied() || this->next || this->prev;
}

void Allocator::MetaInfo::setOccupied(bool occ) {
    if (occ) {
        size |= sign_mask;
    } else {
        size = (size << 1) >> 1;
    }
}

void Allocator::MetaInfo::setSize(size_t sz) {
    bool occ = isOccupied();
    size = sz;
    setOccupied(occ);
}

size_t Allocator::MetaInfo::getSize() const {
    return (size << 1) >> 1;
}
