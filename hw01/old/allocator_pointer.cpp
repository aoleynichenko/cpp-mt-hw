#include "allocator_pointer.h"

#define NULL_PTR_IDX -1

Pointer::Pointer()
	:	idx_(NULL_PTR_IDX), allocator_(nullptr)
{
}

Pointer::Pointer(const Pointer& p)
{
	idx_ = p.idx_;
	allocator_ = p.allocator_;
}

// protected constructor
Pointer::Pointer(size_t idx, Allocator* alc)
	:	idx_(idx), allocator_(alc)
{
}

Pointer::~Pointer()
{
}

Pointer& Pointer::operator=(const Pointer& p)
{
	if (this != &p) {
		idx_ = p.idx_;
		allocator_ = p.allocator_;
	}
	return *this;
}

bool Pointer::operator==(const Pointer& other)
{
	return (idx_ == other.idx_) && (allocator_ == other.allocator_);
}

bool Pointer::operator!=(const Pointer& other)
{
	return !(*this == other);
}

void* Pointer::get() const
{
	return (idx_ == NULL_PTR_IDX) ? nullptr : allocator_->get(idx_);
}

// private methods

void Pointer::reset()
{
	idx_ = NULL_PTR_IDX;
}

// helper functions

bool operator==(const Pointer& p1, void* p2)
{
	return p1.get() == p2;
}

bool operator==(void* p1, const Pointer& p2)
{
	return p1 == p2.get();
}

bool operator!=(const Pointer& p1, void* p2)
{
	return !(p1 == p2);
}

bool operator!=(void* p1, const Pointer& p2)
{
	return !(p1 == p2);
}
