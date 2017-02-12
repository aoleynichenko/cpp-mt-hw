#ifndef ALLOCATOR_POINTER
#define ALLOCATOR_POINTER

#include <stddef.h>

// Forward declaration. Do not include real class definition
// to avoid expensive macros calculations and increase compile speed
#include "allocator.h"

class Pointer {
public:
    Pointer();
    Pointer(const Pointer& p);
    ~Pointer();
    
    Pointer& operator=(const Pointer& p);
    
    void* get() const;
    
    friend class Allocator;
protected:
	Pointer(size_t idx, Allocator* alc);
private:
    size_t     idx_;
    Allocator* allocator_;
    
    void reset();
};

#endif //ALLOCATOR_POINTER
