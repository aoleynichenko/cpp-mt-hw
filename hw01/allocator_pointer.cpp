#include "allocator_pointer.h"
#include <stdio.h>
#include <utility>

Pointer::Pointer(void* p)
	:	self_(p)
{
}

// очень хреновое решение
struct header {
	header* nxt; 
    void**  ptr; 
    size_t  size;
};

Pointer::Pointer(Pointer&& p)
{
//	Allocator::Header* h;
	//printf("move, &p.self_ = %p, ", &p.self_);
	self_ = p.self_;
	p.self_ = nullptr;
	// update header in allocator
	header* h = (header*) self_ - 1;
	h->ptr = &self_;
	//printf("after move, now &self_ = %p\n", &self_);
}

Pointer::~Pointer()
{
}

Pointer& Pointer::operator=(Pointer&& p)
{
	if (this != &p) {
		self_ = p.self_;
		p.self_ = nullptr;
	}
	return *this;
}

void* Pointer::get() const
{
	return self_;
}
