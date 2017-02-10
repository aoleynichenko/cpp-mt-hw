#include "allocator.h"
#include "allocator_error.h"
#include "allocator_pointer.h"

#include <stdio.h>
#include <string.h>
//#define DEBUG

Allocator::Allocator(void* base, size_t size)
    :   base_(base), sz_(size), top_(nullptr)
{
    head_.nxt = freep_ = &head_;
    head_.ptr = nullptr; // no smart pointers
    head_.size = 0;
}

/* We treat the block of data as an array of Header objects.
 * Only first element is used to store information about next free block,
 * other elements (tail) we use as raw bytes (finally we cast it to void *).
 */
Pointer Allocator::alloc(size_t N) {
    Allocator::Header* prevp = freep_;
    Allocator::Header* p;
    //printf("alloc %d bytes\n", N);
    
    size_t nunits = (N + sizeof(Allocator::Header) - 1) / sizeof(Allocator::Header) + 1;
    for (p = prevp->nxt; ; prevp = p, p = p->nxt) {
        if (p->size >= nunits) {     // block is large enough
            if (p->size == nunits)   // exactly required size of block
                prevp->nxt = p->nxt;
            else {   // cut and then return tail bytes
                p->size -= nunits;
                p += p->size;
                p->size = nunits;
            }
            freep_ = prevp;
            Pointer smart((void *) (p+1));
            p->ptr = &smart.self_;
            //printf("&smart.self_ = %p\n", p->ptr);
            return smart;
        }
        if (p == freep_) {   // loop over circled linked list is ended
            //if ((p = add_block(nunits)) == nullptr)
            //    return NULL;
            add_block(nunits);
            p = freep_;
		}
    }
}

void Allocator::realloc(Pointer& p, size_t N)
{
}

void Allocator::free(Pointer& ptr)
{
    Allocator::Header *bp, *p;
    //printf("FREE\n");
    
    bp = (Allocator::Header *) ptr.get() -1; /* указатель на заголовок блока */
    if (bp < base_ || bp >= base_ + sz_)
        throw AllocError(AllocErrorType::InvalidFree, "attempt to free memory at the invalid address");
    
    for (p = freep_; !(bp > p && bp < p->nxt); p = p->nxt)
        if (p >= p->nxt && (bp > p || bp < p->nxt))
            break; /* освобождаем блок в начале или в конце */
    if (bp + bp->size == p->nxt) { /* слить с верхним */
        bp->size += p->nxt->size; /* соседом */
        bp->nxt = p->nxt->nxt;
    }
    else
        bp->nxt = p->nxt;
    if (p + p->size == bp) { /* слить с нижним соседом */
        p->size += bp->size;
        p->nxt = bp->nxt;
    }
    else
        p->nxt = bp;
    
    //printf("self = %p\n", ptr.self_);
    //printf("*bp->ptr = %p\n", *bp->ptr);
    
    //*(bp->ptr) = nullptr;
    *bp->ptr = nullptr;
    bp->ptr = nullptr;   // mark block as unused
    //printf("free bp->ptr = %p\n", bp->ptr);
    //ptr.self_ = nullptr;
    freep_ = p;
}

void Allocator::defrag()
{
    Allocator::Header* start = (Allocator::Header*) base_, *p, *to;
    for (to = p = start; p != top_; p += p->size + 1)
        if (p->ptr) {  // block is in use
            // TODO debug printf
            memmove(to, p, sizeof(Allocator::Header) * (p->size + 1));
            *to->ptr = (void*) (to + 1);
            to += to->size + 1;
        }
    // now 'to' points on first unused Header
    head_.nxt = to;
    to->nxt = &head_;
    to->ptr = nullptr;
    to->size = top_ - to - 1;
    freep_ = to;
}

std::string Allocator::dump() const
{
    return "";
}

// private methods

void Allocator::add_block(size_t nunits)
{
	#ifdef DEBUG
	printf("--------------------------------------\n");
	printf("sizeof Header = %d\n", sizeof(Allocator::Header));
	printf("top_  = %p\n", top_);
	#endif
	
    if (top_ == nullptr) {  // no blocks yet
		//printf("first time\n");
        top_ = (Header*) base_;
    }
    // проматываем указатель на свободные блоки до момента, пока он еще меньше top
    Allocator::Header* prevp = freep_;
    Allocator::Header* p;
    
    for (p = prevp->nxt; p < top_; prevp = p, p = p->nxt) {
        //printf("loop\n");
    }
    
    #ifdef DEBUG
    printf("base_ = %p\n", base_);
    printf("top_  = %p\n", top_);
    printf("dist  = %d\n", top_ - (Header*)base_);
    #endif
    
    if ((char*) (top_ + nunits) > (char*) base_ + sz_)
		throw AllocError(AllocErrorType::NoMemory, "not enough memory");
    
    //top_->nxt = head_.nxt;
    top_->size = nunits;
    top_->ptr = nullptr; // this block is unused now
    top_->nxt = &head_;
    prevp->nxt = top_;
    freep_ = top_;
    top_ += nunits + 1;
    
    #ifdef DEBUG
    printf("new block size %d allocated\n", nunits+1);
    printf("base_ = %p\n", base_);
    printf("top_  = %p\n", top_);
    printf("dist  = %d\n", top_ - (Header*) base_);
    
    getchar();
    #endif
}






