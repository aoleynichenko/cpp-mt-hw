#include "allocator.h"
#include "allocator_error.h"
#include "allocator_pointer.h"

Allocator::Allocator(void* base, size_t size)
  : buf(base), bufsize(size), head(nullptr)
{
    meta = (MetaInfo*) (buf + bufsize);
    size_t sz = ((char*) meta - (char*) buf) / sizeof(void*);
    addFree(buf, sz);
}

Pointer Allocator::alloc(size_t N) {
    size_t nunits = (N + sizeof(void*) - 1) / sizeof(void*) + 1;

    for (MetaInfo* p = head; p != nullptr; p = p->next) {
        if (p->size == nunits) {
            p->offs *= -1; // free -> occupied
            MetaInfo* info = unlinkFree(p);
            return Pointer(indexOfInfo(info), this);
        }
        else if (p->size >= nunits) {
            MetaInfo* info = unlinkFree(p);
            p->size = nunits;
            void* mem = (char*) buf + p->offs;
            *((MetaInfo**) mem) = p;
            addFree(mem + nunits*sizeof(void*), nunits);
            return Pointer(indexOfInfo(info), this);
        }
    }
    throw AllocError(AllocErrorType::NoMemory, "not enough memory");
}

std::string dump() const
{
    
    return "";
}

// private methods

Allocator::MetaInfo* Allocator::addFree(void* start, size_t nunits)
{
    meta--;
    meta->size = nunits;
    meta->offs = (char*) start - (char*) buf;
    meta->next = head;
    meta->prev = nullptr;
    head = meta;
}

Allocator::MetaInfo* Allocator::unlinkFree(Allocator::MetaInfo* info)
{
    if (info->prev) {
        info->prev->next = info->next;
    }
    if (info->next) {
        info->next->prev = info->prev;
    }
    return info;
}

void* Allocator::get(size_t idx)
{
    MetaInfo* end = (MetaInfo*) (buf + bufsize);
    MetaInfo* info = end - idx - 1;
    return (char*) buf + info->offs + sizeof(void*);
}

size_t Allocator::indexOfInfo(Allocator::MetaInfo* info)
{
    MetaInfo* end = (MetaInfo*) (buf + bufsize);
    size_t index = end - info - 1;
    return index;
}
