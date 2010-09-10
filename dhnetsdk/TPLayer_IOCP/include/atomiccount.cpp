
#include "atomiccount.h"

NAMESPACE_BEGIN(SPTools)

#if defined(OS_LINUX)

#include "atomic.h"

/*
inline int mx_atomic_set_int(volatile int *ptr, int newval) {
    asm volatile("xchgl %0,%1"
            : "=r" (newval), "+m" (*ptr) 
            : "0" (newval)
            : "memory"); 
    return newval;
}

inline int mx_atomic_increment(volatile int *ptr) { 
    unsigned char ret; 
    asm volatile("lock\n" 
            "incl %0\n"
            "setne %1" 
            : "=m" (*ptr), "=qm" (ret)
            : "m" (*ptr) 
            : "memory"); 
    return static_cast<int>(ret);
}

inline int mx_atomic_decrement(volatile int *ptr) {
    unsigned char ret;    
    asm volatile("lock\n"
            "decl %0\n"
            "setne %1" 
            : "=m" (*ptr), "=qm" (ret)
            : "m" (*ptr) 
            : "memory"); 
    return static_cast<int>(ret);
}

typedef struct atomic_count_t {

  volatile int count;

  atomic_count_t() {
    mx_atomic_set_int(&count, 0);
  }

  ~atomic_count_t() {
  }

} ATOMIC_COUNT;
*/


AtomicCount::AtomicCount() {
	_value = reinterpret_cast<void*>(new atomic_t);

//  _value = reinterpret_cast<void*>(new ATOMIC_COUNT);
  
}

AtomicCount::~AtomicCount() {
	delete reinterpret_cast<atomic_t*>(_value);

//  delete reinterpret_cast<ATOMIC_COUNT*>(_value);

}
  
bool AtomicCount::ref() {
	return atomic_add_return(1, reinterpret_cast<atomic_t*>(_value)) > 0;

//  return mx_atomic_increment(&reinterpret_cast<ATOMIC_COUNT*>(_value)->count) != 0;
  
}
  
bool AtomicCount::deref() {
	return atomic_sub_return(1, reinterpret_cast<atomic_t*>(_value)) <= 0;

//  return mx_atomic_decrement(&reinterpret_cast<ATOMIC_COUNT*>(_value)->count) == 0;
  
}

#elif defined(OS_WIN32)

AtomicCount::AtomicCount() 
{
  _value = reinterpret_cast<void*>(new LONG(0));
}

AtomicCount::~AtomicCount() 
{
  delete reinterpret_cast<LPLONG>(_value);
}
  
bool AtomicCount::ref() {
  LONG v = ::InterlockedIncrement(reinterpret_cast<LPLONG>(_value));
  return static_cast<unsigned long>(v) != 0;
}
  
bool AtomicCount::deref() {

  LONG v = ::InterlockedDecrement(reinterpret_cast<LPLONG>(_value));
  return static_cast<unsigned long>(v) == 0;
  
}

#endif

NAMESPACE_END

