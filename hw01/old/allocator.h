#ifndef ALLOCATOR
#define ALLOCATOR
#include <string>

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
    void realloc(Pointer& p, size_t N);

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
    std::string dump() const;
    
    friend class Pointer;
private:
    void* base_;
    size_t sz_;
    
    /** I use idea from K&R 8.7 to represent free blocks of bytes which
     *  can be allocated.
     */
    struct Header {
        Header*  nxt;   // next free block of memory
        Header** idx;   // for indirect addressing
        size_t   size;  // block size (including size of header subblock)
    };
    
    Header  head_;
    Header* freep_;    // list of free blocks
    Header* top_;      // last added block (may be occupied)
    
    // add new block of (nunits+1)*sizeof(Header) bytes to the end
    // (last address!) of the single-linked list
    void add_block(size_t nunits);
    size_t add_block_info(Header** h);
    void* get(size_t idx);
    
    // block with indirect addresses
    // last element has address ind_top_
    Header** ind_top_;
    Header** ind_curr_;
};

#endif // ALLOCATOR
