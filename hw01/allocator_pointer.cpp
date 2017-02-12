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

void* Pointer::get() const
{
	return (idx_ == NULL_PTR_IDX) ? nullptr : allocator_->get(idx_);
}

// private methods

void Pointer::reset()
{
	idx_ = NULL_PTR_IDX;
}
