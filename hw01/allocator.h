#ifndef ALLOCATOR
#define ALLOCATOR
#include <string>
#include <stdint.h>

// Forward declaration. Do not include real class definition
// to avoid expensive macros calculations and increase compile speed
class Pointer;

/**
 * Wraps given memory area and provides defagmentation allocator interface on
 * the top of it.
 *
 *
 */
class Allocator {
public:
    Allocator(void* base, size_t size);

    /**
     * TODO: semantics
     * @param N size_t
     */
    Pointer alloc(size_t N);

    /**
     * TODO: semantics
     * @param p Pointer
     * @param N size_t
     */
    void realloc(Pointer& p, size_t N) {}

    /**
     * TODO: semantics
     * @param p Pointer
     */
    void free(Pointer& p);

    /**
     * TODO: semantics
     */
    void defrag();

    /**
     * TODO: semantics
     */
    std::string dump();// const;

    friend class Pointer;
private:

    struct MetaInfo {
      MetaInfo* next;
      MetaInfo* prev;
      int64_t  size;
      size_t   offs;

      MetaInfo();
    };

    MetaInfo head;
    MetaInfo* meta;
    void*  buf;
    size_t bufsize;

    MetaInfo* addFree(void* start, size_t nbytes);
    MetaInfo* addMeta(void* start, size_t nbytes);
    MetaInfo* unlinkFree(MetaInfo* info);
    void* get(size_t idx);
    size_t indexOfInfo(MetaInfo* info);
    MetaInfo* getMeta(size_t idx);
    MetaInfo* insertFree(MetaInfo* info);
};

#endif // ALLOCATOR
