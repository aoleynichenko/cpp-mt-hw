#ifndef ALLOCATOR_POINTER
#define ALLOCATOR_POINTER

#include <stddef.h>
#include "allocator.h"

class Pointer {
public:
    Pointer();
    Pointer(const Pointer&);
    ~Pointer();

    Pointer& operator=(const Pointer&);
    bool operator==(const Pointer&);
    bool operator!=(const Pointer&);
    operator bool() const;

    void* get() const;

    friend class Allocator;
protected:
	Pointer(size_t idx, Allocator* alc);
private:
    size_t     idx_;
    Allocator* allocator_;

    void reset();
};

bool operator==(const Pointer&, void*);
bool operator==(void*, const Pointer&);
bool operator!=(const Pointer&, void*);
bool operator!=(void*, const Pointer&);

#endif //ALLOCATOR_POINTER
