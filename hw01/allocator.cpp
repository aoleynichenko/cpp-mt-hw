#include "allocator.h"
#include "allocator_error.h"
#include "allocator_pointer.h"

#include <stdio.h>
#include <string.h>

Allocator::Allocator(void* base, size_t size)
    :   buf(base), bufsize(size)
{
    meta = (MetaInfo*) (buf + bufsize);
    size_t sz = ((char*) meta - (char*) buf) / sizeof(void*);
    addFree(buf, sz);
    head.next->size = -((char*) meta - (char*) buf) / sizeof(void*);
    printf("head.next->size = %d 0x%X\n", head.next->size, head.next->size);
    printf("0s = 0x%X\n", 0L);
    printf("-1 = 0x%X\n", -1);
    printf("head.next->size > 0: %d\n", (head.next->size < 0));
}

Pointer Allocator::alloc(size_t N) {
    size_t nunits = (N + sizeof(void*) - 1) / sizeof(void*) + 1;

    for (MetaInfo* p = head.next; p != nullptr; p = p->next) {
        if (abs(p->size) == nunits) {
            p->size *= -1; // free -> occupied
            MetaInfo* info = unlinkFree(p);
            return Pointer(indexOfInfo(info), this);
        }
        else if (abs(p->size) >= nunits) {
            // we return block of sizeof(void*)*bytes
            // tail remains free, we update its metainfo
            void* mem = (char*) buf + p->offs;
            MetaInfo* occ_meta = addMeta(mem, nunits);

            void* mem_free = (char*) buf + p->offs + nunits * sizeof(void*);
            *((MetaInfo**) mem_free) = p;
            p->size += nunits;
            p->offs = (char*) mem_free - (char*) buf;

            return Pointer(indexOfInfo(occ_meta), this);
        }
    }
    throw AllocError(AllocErrorType::NoMemory, "not enough memory");
}

void Allocator::free(Pointer& p)
{
    MetaInfo* info = getMeta(p.idx_);
    info->size *= -1;
    insertFree(info);
    p.reset();
}
#include <limits.h>
void Allocator::defrag()
{
    MetaInfo* last = getMeta(0);
    char* lim = (char*) buf + last->offs + sizeof(void*) * abs(last->size);
    char* p, *to;
    for (to = p = (char*) buf; p < lim; ) {
        MetaInfo* hd = *((MetaInfo**) p);
        int offs = p - (char*) buf;
        printf("SIZE=%d %u  ", hd->size, hd->size);
        /*int64_t zero = 0;
        printf(" > 0: %d  ", hd->size > zero);*/
        if (/*hd->size > 0*/(!hd->prev) && (!hd->next)) {
            printf("move from offs %d  size = %d\n", offs, hd->size);
            printf("hd->prev = %p   hd->next = %p\n", hd->prev, hd->next);
            memmove(to, p, abs(hd->size)*sizeof(void*));
            *((MetaInfo**)to) = hd;
            to += abs(hd->size)*sizeof(void*);
        }
        else printf("ignore\n");
        p += abs(hd->size) * sizeof(void*);
    }
    last->size = -((char*) meta - to)/sizeof(void*);
    head.next = last;
    last->prev = &head;
}

std::string Allocator::dump()// const
{
    printf("\n================================ DUMP ==============================\n");
    Allocator::MetaInfo* end = (Allocator::MetaInfo*) (buf + bufsize);
    int i = 1;
    printf("head.next = %p\n", head.next);
    printf("&head = %p\n", &head);
    for (auto p = meta; p != end; p++, i++) {
        printf("%d   %p   { offs=%d prev=%p next=%p size=%d}\n", i, p, p->offs, p->prev, p->next, p->size);
    }
    printf("--------------------------------------------------------------------\n");
    MetaInfo* last = getMeta(0);
    char* lim = (char*) buf + last->offs + sizeof(void*) * abs(last->size);
    for (char* p = (char*) buf; p < lim; ) {
        MetaInfo* hd = *((MetaInfo**) p);
        int offs = p - (char*) buf;
        printf("%d  addr metainf = %p   size = %d\n", offs, hd, hd->size);
        p += abs(hd->size) * sizeof(void*);
    }
    printf("====================================================================\n\n");
    return "";
}

Allocator::MetaInfo::MetaInfo()
    :   next(nullptr), prev(nullptr), size(0), offs(0)
{
}

// private methods
// ВСЕ ДЕЛО В УКАЗАТЕЛЕ HEAD! unlink не затирает в нем данные!
Allocator::MetaInfo* Allocator::addFree(void* start, size_t nunits)
{
  /*MetaInfo* last = getMeta(0);
  char* lim = (char*) buf + last->offs + sizeof(void*) * abs(last->size);
  if (lim >= (char*)(meta-1))
    throw AllocError(AllocErrorType::NoMemory, "not enough memory for metainfo");
    last->size -= sizeof(MetaInfo) / sizeof(void*);
*/
    meta--;
    meta->size = (-1) * nunits;
    meta->offs = (char*) start - (char*) buf;
    *((MetaInfo**) start) = meta;

    return insertFree(meta);
}

Allocator::MetaInfo* Allocator::addMeta(void* start, size_t nunits)
{
    /*printf("add meta\n");
    printf("meta = %p\n", meta);*/

    MetaInfo* last = getMeta(0);
//    printf("last = %p\n", last);
    char* lim = (char*) buf + last->offs + sizeof(void*) * abs(last->size);
//    printf("last size = %d\n", last->size);
//    printf("lim = %p (%s meta)\n", lim, (lim < (char*) meta) ? "<" : ">=");
  /*  if (lim >= (char*)(meta-1))
      throw AllocError(AllocErrorType::NoMemory, "not enough memory for metainfo");
*/

    meta--;
    last->size = (-1) * ((char*) meta - ((char*)buf + last->offs)) / sizeof(void*);
//    printf("last->size = %d\n", last->size);
    meta->size = nunits;
    meta->offs = (char*) start - (char*) buf;
    *((MetaInfo**) start) = meta;
    return meta;
}

Allocator::MetaInfo* Allocator::unlinkFree(Allocator::MetaInfo* info)
{
    //printf("unlink free. info->prev = %p  info->next = %p\n", info->prev, info->next);
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

Allocator::MetaInfo* Allocator::getMeta(size_t idx)
{
    MetaInfo* end = (MetaInfo*) (buf + bufsize);
    MetaInfo* info = end - idx - 1;
    return info;
}


// insert metainfo to the beginning of double-linking list
Allocator::MetaInfo* Allocator::insertFree(Allocator::MetaInfo* info)
{
    printf("insert free\n");
    if (head.next) {
        head.next->prev = info;
    }
    info->next = head.next;
    info->prev = &head;
    head.next = info;

    return info;
}
