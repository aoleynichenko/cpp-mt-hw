#ifndef ALLOCATOR_POINTER
#define ALLOCATOR_POINTER

// Forward declaration. Do not include real class definition
// to avoid expensive macros calculations and increase compile speed
class Allocator;

class Pointer {
public:
    Pointer(void* p = nullptr);
    Pointer(Pointer&& p);
    ~Pointer();
    
    Pointer& operator=(Pointer&& p);
    
    void* get() const;
    
    friend class Allocator;
protected:
    Pointer(const Pointer& p);
    Pointer& operator=(const Pointer& p);
private:
    void* self_;
};

#endif //ALLOCATOR_POINTER
