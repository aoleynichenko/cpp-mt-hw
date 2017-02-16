#include "allocator.h"
#include "allocator_error.h"
#include "allocator_pointer.h"

#include <sstream>
#include <stdio.h>
#include <string.h>
//#define DEBUG

Allocator::Allocator(void* base, size_t size)
    :   base_(base), sz_(size), top_(nullptr)
{
    head_.nxt = freep_ = &head_;
    head_.idx = nullptr;
    head_.size = 0;
    // pointers for auxiliary data
    ind_top_ = (Allocator::Header**) (base + size);
    ind_curr_ = ind_top_;
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
            //printf("block found! p = %p, p->size = %d, nunits = %d\n", p, p->size, nunits);
            if (p->size == nunits)   // exactly required size of block
                prevp->nxt = p->nxt;
            else {   // cut and then return first bytes
                /*p->size -= nunits;
                p += p->size;
                p->size = nunits;*/
                Header* newp = p + p->size;
                newp->nxt = p->nxt;
                prevp->nxt = newp;
                newp->size = nunits - p->size;
                newp->idx = nullptr;
                p->size = nunits;
            }
            freep_ = prevp;
            //printf("freep = %p\n", freep_);
            // add information about this block to the array for indirect addressing
            // now p points to the header of the current (now occupied) block
            size_t idx = add_block_info(&p);
            Pointer smart(idx, this);
            
            //dump();
            
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
    /*dump();
    if (p.get())
        this->free(p);
    dump();
    p = this->alloc(N);
    dump();*/
    // get block header
    Allocator::Header *bp = (Allocator::Header *) p.get() - 1;
    size_t nunits = (N + sizeof(Allocator::Header) - 1) / sizeof(Allocator::Header) + 1;
    
    if (p.get() == nullptr) {
        p = this->alloc(N);
        return;
    }
    else if (nunits <= bp->size) { // shrink
        this->free(p);
        p = this->alloc(N);
        return;
    }
    
    // now we move to the first occupied block
    // if not enough memory, we will do simple alloc()
    // else me will merge all free block to one   
    this->free(p); 
    size_t count = 0;
    //printf("has: %d\n", bp->size);
    //printf("required: %d\n", nunits);
    //dump();
    
    Allocator::Header* lastfree = bp, *ptr;
    for (ptr = bp; ptr != top_ && count < nunits/* && ptr->idx != nullptr*/; lastfree = ptr, ptr += ptr->size) {
        if (ptr->idx)
            break;
        count += ptr->size;
        //printf("count = %d\n", count);
    }
    if (ptr == top_) {
        //printf("top!\n");
        size_t required = nunits - count;
        add_block(required);
        bp->nxt = &head_;
        bp->size = nunits;
        bp->idx = nullptr;
        freep_ = bp;
        //dump();
        p = alloc(N);
    }
    else if (ptr != top_ && count < nunits) {
        // we move data here!!!
        p = alloc(N);
        memmove(p.get(), bp+1, (nunits-1)*sizeof(Allocator::Header));
    }
    else { // merge internal blocks
        bp->nxt = lastfree->nxt;
        bp->idx = nullptr;
        bp->size = count;
        freep_ = bp;
        p = alloc(N);
    }
    

    
    /*else if (bp + bp->size == top_) { // grow memory for the last element
        size_t required = nunits - bp->size;
        // we create new free block with block->size == nunits
        add_block(required);
        bp->nxt = &head_;
    }*/
}

void Allocator::free(Pointer& ptr)
{
    Allocator::Header *bp, *p;
    //printf("FREE\n");
    
    bp = (Allocator::Header *) ptr.get() - 1; /* указатель на заголовок блока */
    if (bp < base_ || bp >= base_ + sz_)
        throw AllocError(AllocErrorType::InvalidFree, "attempt to free memory at the invalid address");
    
    *(bp->idx) = nullptr; // remove indirect address
    bp->idx = nullptr;    // this block is free now
    ptr.reset();
    
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
    //*bp->ptr = nullptr;
    //bp->ptr = nullptr;   // mark block as unused
    //printf("free bp->ptr = %p\n", bp->ptr);
    //ptr.self_ = nullptr;
    //p->idx = nullptr;
    freep_ = bp;  // points to freed block
}

void Allocator::defrag()
{
    //dump();
    
    Allocator::Header* start = (Allocator::Header*) base_, *p, *to;
    for (to = p = start; p != top_; p += p->size)
        if (p->idx) {  // block is in use
            // TODO debug printf
            memmove(to, p, sizeof(Allocator::Header) * (p->size));
            *(to->idx) = to;
            to += to->size;
        }
    // now 'to' points on first unused Header
    head_.nxt = to;
    to->nxt = &head_;
    to->idx = nullptr;
    to->size = top_ - to;
    freep_ = to;
    
    //dump();
}

std::string Allocator::dump() const
{
    Allocator::Header* start = (Allocator::Header*) base_, *p;
    
    printf("\n============================= DUMP ============================\n");
    printf("&head    %p\n", &head_);
    printf("head.nxt %p\n", head_.nxt);
    printf("top      %p\n", top_);
    printf("freep    %p\n", freep_);
    printf("---------------------------------------------------------------\n");
    printf("  addr              next             idx            size\n");
    for (p = start; p != top_; p += p->size) {
        printf("%-12p [ %-16p | %-16p | %-2d ]\n", p, p->nxt, p->idx, p->size);
    }
    printf("%p\n", p);
    printf("===============================================================\n\n");
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
        top_->idx = nullptr;
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
    
    if ((void*)(top_ + nunits) >= (void*)ind_top_)
		throw AllocError(AllocErrorType::NoMemory, "not enough memory");
    
    //top_->nxt = head_.nxt;
    top_->size = nunits;
    //top_->ptr = nullptr; // this block is unused now
    top_->nxt = &head_;
    top_->idx = nullptr;
    prevp->nxt = top_;
    freep_ = top_;
    top_ += nunits;
    
    #ifdef DEBUG
    printf("new block size %d allocated\n", nunits+1);
    printf("base_ = %p\n", base_);
    printf("top_  = %p\n", top_);
    printf("dist  = %d\n", top_ - (Header*) base_);
    
    getchar();
    #endif
}

size_t Allocator::add_block_info(Header** h)
{
    /*// search for empty cell
    // loop over all cells, beginning from last used cell
    for (Header* p = ind_curr_; p < base_ + sz_; p--) {
        
        // go to begin
        if (p == ind_top_)
            p = 
    }*/
    
    //dump();
    
    //printf("add block info\n");
    Header** end = (Header**) (base_ + sz_);
    if ((void*) ind_top_ == (void*) top_)
        throw AllocError(AllocErrorType::NoMemory, "not enough memory to story auxiliary information");
    
    ind_top_--;
    Header** p = ind_top_;
    (*h)->idx = p;
    *p = *h;
    size_t idx = end - p - 1;
    
    /*printf("end = %p\n", end);
    printf("idx = %d\n", idx);
    printf("p = %p\n", p);
    printf("*p = %p\n", *p);*/
    
    //dump();
    
    return idx;
}

void* Allocator::get(size_t idx)
{
    //printf("get idx = %d\n", idx);
    Header** end = (Header**) (base_ + sz_);
    //printf("end = %p\n", end);
    //printf("end-1 = %p\n", end-1);
    Header** p = end - idx - 1;
    //printf("p = %p\n", p);
    return (void*) (*p + 1);
}



