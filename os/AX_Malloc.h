#ifndef _AX_MALLOC_H
#define _AX_MALLOC_H

#include "AX_OS.h"
#include <assert.h>

class AX_Allocator
{
public:
	// = Memory Management

	/// Get pointer to a default ACE_Allocator.
	static AX_Allocator *instance (void);

	/// Set pointer to a process-wide ACE_Allocator and return existing
	/// pointer.
	static AX_Allocator *instance (AX_Allocator *);

	/// Delete the dynamically allocated Singleton
	static void close_singleton (void);

	/// "No-op" constructor (needed to make certain compilers happy).
	AX_Allocator (void);

	/// Virtual destructor
	virtual ~AX_Allocator (void);

	/// Allocate <nbytes>, but don't give them any initial value.
	virtual void *malloc (size_t nbytes) = 0;

	/// Allocate <nbytes>, giving them <initial_value>.
	virtual void *calloc (size_t nbytes, char initial_value = '\0') = 0;

	/// Allocate <n_elem> each of size <elem_size>, giving them
	/// <initial_value>.
	virtual void *calloc (size_t n_elem,
		size_t elem_size,
		char initial_value = '\0') = 0;

	/// Free <ptr> (must have been allocated by <ACE_Allocator::malloc>).
	virtual void free (void *ptr) = 0;

	/// Remove any resources associated with this memory manager.
	virtual int remove (void) = 0;

	// = Map manager like functions

	/**
	* Associate <name> with <pointer>.  If <duplicates> == 0 then do
	* not allow duplicate <name>/<pointer> associations, else if
	* <duplicates> != 0 then allow duplicate <name>/<pointer>
	* assocations.  Returns 0 if successfully binds (1) a previously
	* unbound <name> or (2) <duplicates> != 0, returns 1 if trying to
	* bind a previously bound <name> and <duplicates> == 0, else
	* returns -1 if a resource failure occurs.
	*/
	virtual int bind (const char *name, void *pointer, int duplicates = 0) = 0;

	/**
	* Associate <name> with <pointer>.  Does not allow duplicate
	* <name>/<pointer> associations.  Returns 0 if successfully binds
	* (1) a previously unbound <name>, 1 if trying to bind a previously
	* bound <name>, or returns -1 if a resource failure occurs.  When
	* this call returns <pointer>'s value will always reference the
	* void * that <name> is associated with.  Thus, if the caller needs
	* to use <pointer> (e.g., to free it) a copy must be maintained by
	* the caller.
	*/
	virtual int trybind (const char *name, void *&pointer) = 0;

	/// Locate <name> and pass out parameter via pointer.  If found,
	/// return 0, returns -1 if failure occurs.
	virtual int find (const char *name, void *&pointer) = 0;

	/// Returns 0 if the name is in the mapping. -1, otherwise.
	virtual int find (const char *name) = 0;

	/// Unbind (remove) the name from the map.  Don't return the pointer
	/// to the caller
	virtual int unbind (const char *name) = 0;

	/// Break any association of name.  Returns the value of pointer in
	/// case the caller needs to deallocate memory.
	virtual int unbind (const char *name, void *&pointer) = 0;

	// = Protection and "sync" (i.e., flushing memory to persistent
	// backing store).

	/**
	* Sync @a len bytes of the memory region to the backing store
	* starting at @c this->base_addr_.  If @a len == -1 then sync the
	* whole region.
	*/
	virtual int sync (ssize_t len = -1, int flags = 0) = 0;

	/// Sync @a len bytes of the memory region to the backing store
	/// starting at @a addr.
	virtual int sync (void *addr, size_t len, int flags = 0) = 0;

	/**
	* Change the protection of the pages of the mapped region to <prot>
	* starting at <this->base_addr_> up to <len> bytes.  If <len> == -1
	* then change protection of all pages in the mapped region.
	*/
	virtual int protect (ssize_t len = -1, int prot = PROT_RDWR) = 0;

	/// Change the protection of the pages of the mapped region to <prot>
	/// starting at <addr> up to <len> bytes.
	virtual int protect (void *addr, size_t len, int prot = PROT_RDWR) = 0;

#if defined (ACE_HAS_MALLOC_STATS)
	/// Dump statistics of how malloc is behaving.
	virtual void print_stats (void) const = 0;
#endif /* ACE_HAS_MALLOC_STATS */

	/// Dump the state of the object.
	virtual void dump (void) const = 0;
private:
	// DO NOT ADD ANY STATE (DATA MEMBERS) TO THIS CLASS!!!!  See the
	// <ACE_Allocator::instance> implementation for explanation.

	/// Pointer to a process-wide ACE_Allocator instance.
	static AX_Allocator *allocator_;

	/// Must delete the <allocator_> if non-0.
	static int delete_allocator_;
};

class AX_Static_Allocator_Base : public AX_Allocator
{
public:
	AX_Static_Allocator_Base (char *buffer, size_t size);
	virtual void *malloc (size_t nbytes);
	virtual void *calloc (size_t nbytes, char initial_value = '\0');
	virtual void *calloc (size_t n_elem, size_t elem_size, char initial_value = '\0');
	virtual void free (void *ptr);
	virtual int remove (void);
	virtual int bind (const char *name, void *pointer, int duplicates = 0);
	virtual int trybind (const char *name, void *&pointer);
	virtual int find (const char *name, void *&pointer);
	virtual int find (const char *name);
	virtual int unbind (const char *name);
	virtual int unbind (const char *name, void *&pointer);
	virtual int sync (ssize_t len = -1, int flags = 0);
	virtual int sync (void *addr, size_t len, int flags = 0);
	virtual int protect (ssize_t len = -1, int prot = PROT_RDWR);
	virtual int protect (void *addr, size_t len, int prot = PROT_RDWR);
#if defined (ACE_HAS_MALLOC_STATS)
	virtual void print_stats (void) const;
#endif /* ACE_HAS_MALLOC_STATS */
	virtual void dump (void) const;

protected:
	/// Don't allow direct instantiations of this class.
	AX_Static_Allocator_Base (void);

	/// Pointer to the buffer.
	char *buffer_;

	/// Size of the buffer.
	size_t size_;

	/// Pointer to the current offset in the <buffer_>.
	size_t offset_;
};
/*
* @class ACE_New_Allocator
*
* @brief Defines a class that provided a simple implementation of
* memory allocation.
*
* This class uses the new/delete operators to allocate and free up
* memory.  Please note that the only methods that are supported are
* <malloc>, <calloc>, and <free>. All other methods are no-ops that
* return -1 and set <errno> to <ENOTSUP>.  If you require this
* functionality, please use: ACE_Allocator_Adapter <ACE_Malloc
* <ACE_LOCAL_MEMORY_POOL, MUTEX> >, which will allow you to use the
* added functionality of bind/find/etc. while using the new/delete
* operators.
*/
class AX_New_Allocator : public AX_Allocator
{
public:
	/// These methods are defined.
	virtual void *malloc (size_t nbytes);
	virtual void *calloc (size_t nbytes, char initial_value = '\0');
	virtual void *calloc (size_t n_elem, size_t elem_size, char initial_value = '\0');
	virtual void free (void *ptr);

	/// These methods are no-ops.
	virtual int remove (void);
	virtual int bind (const char *name, void *pointer, int duplicates = 0);
	virtual int trybind (const char *name, void *&pointer);
	virtual int find (const char *name, void *&pointer);
	virtual int find (const char *name);
	virtual int unbind (const char *name);
	virtual int unbind (const char *name, void *&pointer);
	virtual int sync (ssize_t len = -1, int flags = 0);
	virtual int sync (void *addr, size_t len, int flags = 0);
	virtual int protect (ssize_t len = -1, int prot = PROT_RDWR);
	virtual int protect (void *addr, size_t len, int prot = PROT_RDWR);
#if defined (ACE_HAS_MALLOC_STATS)
	virtual void print_stats (void) const;
#endif /* ACE_HAS_MALLOC_STATS */
	virtual void dump (void) const;

private:
	// DO NOT ADD ANY STATE (DATA MEMBERS) TO THIS CLASS!!!!  See the
	// <ACE_Allocator::instance> implementation for explanation.
};

#include "AX_Malloc.inl"


# define AX_NEW_MALLOC_RETURN(POINTER,ALLOCATOR,CONSTRUCTOR,RET_VAL) \
	   do { POINTER = ALLOCATOR; \
		        if (POINTER == 0) { return RET_VAL;} \
		        else { (void) new (POINTER) CONSTRUCTOR; } \
		      } while (0)
# define AX_NEW_MALLOC(POINTER,ALLOCATOR,CONSTRUCTOR) \
	   do { POINTER = ALLOCATOR; \
		        if (POINTER == 0) { return;} \
		        else { (void) new (POINTER) CONSTRUCTOR; } \
		      } while (0)
# define AX_NEW_MALLOC_NORETURN(POINTER,ALLOCATOR,CONSTRUCTOR) \
	   do { POINTER = ALLOCATOR; \
		        if (POINTER == 0) { } \
		        else { (void) new (POINTER) CONSTRUCTOR; } \
		      } while (0)

# define AX_DES_NOFREE(POINTER,CLASS) \
   do { \
        if (POINTER) \
          { \
            (POINTER)->~CLASS (); \
          } \
      } \
   while (0)

# define AX_DES_ARRAY_NOFREE(POINTER,SIZE,CLASS) \
   do { \
        if (POINTER) \
          { \
            for (size_t i = 0; \
                 i < SIZE; \
                 ++i) \
            { \
              (&(POINTER)[i])->~CLASS (); \
            } \
          } \
      } \
   while (0)

# define AX_DES_FREE(POINTER,DEALLOCATOR,CLASS) \
   do { \
        if (POINTER) \
          { \
            (POINTER)->~CLASS (); \
            DEALLOCATOR (POINTER); \
          } \
      } \
   while (0)

# define AX_DES_ARRAY_FREE(POINTER,SIZE,DEALLOCATOR,CLASS) \
   do { \
        if (POINTER) \
          { \
            for (size_t i = 0; \
                 i < SIZE; \
                 ++i) \
            { \
              (&(POINTER)[i])->~CLASS (); \
            } \
            DEALLOCATOR (POINTER); \
          } \
      } \
   while (0)

#endif

