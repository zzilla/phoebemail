#include "AX_Malloc.h"


AX_Allocator *
AX_Allocator::instance (void)
{
	//  ACE_TRACE ("ACE_Allocator::instance");

	if (AX_Allocator::allocator_ == 0)
	{
		// Perform Double-Checked Locking Optimization.
		//ACE_MT (ACE_GUARD_RETURN (ACE_Recursive_Thread_Mutex, ace_mon,
		//	*ACE_Static_Object_Lock::instance (), 0));

		if (AX_Allocator::allocator_ == 0)
		{
			// Have a seat.  We want to avoid ever having to delete the
			// ACE_Allocator instance, to avoid shutdown order
			// dependencies.  ACE_New_Allocator never needs to be
			// destroyed:  its destructor is empty and its instance
			// doesn't have any state.  Therefore, sizeof
			// ACE_New_Allocator is equal to sizeof void *.  It's
			// instance just contains a pointer to its virtual function
			// table.
			//
			// So, we allocate space for the ACE_New_Allocator instance
			// in the data segment.  Because its size is the same as
			// that of a pointer, we allocate it as a pointer so that it
			// doesn't get constructed statically.  We never bother to
			// destroy it.
			static void *allocator_instance = 0;

			// Check this critical assumption.  We put it in a variable
			// first to avoid stupid compiler warnings that the
			// condition may always be true/false.
#         if !defined (ACE_NDEBUG)
			int assertion = (sizeof allocator_instance ==
				sizeof (AX_New_Allocator));
			assert (assertion);
#         endif /* !ACE_NDEBUG */

			// Initialize the allocator_instance by using a placement
			// new.
			//AX_Allocator::allocator_ =
			//	new (&allocator_instance) AX_New_Allocator;
			AX_Allocator::allocator_ =
				new  AX_New_Allocator;
		}
	}

	return AX_Allocator::allocator_;
}

AX_Allocator *
AX_Allocator::instance (AX_Allocator *r)
{
	//printf ("ACE_Allocator::instance");
	//ACE_GUARD_RETURN (AX_Thread_Mutex, ace_mon,
	//	*ACE_Static_Object_Lock::instance (), 0);
	AX_Allocator *t = AX_Allocator::allocator_;

	// We can't safely delete it since we don't know who created it!
	AX_Allocator::delete_allocator_ = 0;

	AX_Allocator::allocator_ = r;
	return t;
}

void
AX_Allocator::close_singleton (void)
{
	//printf ("ACE_Allocator::close_singleton");

	//ACE_GUARD (ACE_Recursive_Thread_Mutex, ace_mon,
	//	*ACE_Static_Object_Lock::instance ());

	if (AX_Allocator::delete_allocator_)
	{
		// This should never be executed....  See the
		// ACE_Allocator::instance (void) method for an explanation.
		delete AX_Allocator::allocator_;
		AX_Allocator::allocator_ = 0;
		AX_Allocator::delete_allocator_ = 0;
	}
}

AX_Allocator::~AX_Allocator (void)
{
	//printf ("ACE_Allocator::~ACE_Allocator");
}

AX_Allocator::AX_Allocator (void)
{
	//printf ("ACE_Allocator::ACE_Allocator");
}

/******************************************************************************/

void *
AX_New_Allocator::malloc (size_t nbytes)
{
	char *ptr = 0;
		
	AX_NEW_RETURN (ptr, char[nbytes], 0);
	return (void *) ptr;
}

void *
AX_New_Allocator::calloc (size_t nbytes,
						   char initial_value)
{
	char *ptr = 0;

	AX_NEW_RETURN (ptr, char[nbytes], 0);

	memset (ptr, initial_value, nbytes);
	return (void *) ptr;
}

void *
AX_New_Allocator::calloc (size_t n_elem, size_t elem_size, char initial_value)
{
	return AX_New_Allocator::calloc (n_elem * elem_size, initial_value);
}

void
AX_New_Allocator::free (void *ptr)
{
	delete [] (char *) ptr;
}

int
AX_New_Allocator::remove (void)
{
	return (-1);
}

int
AX_New_Allocator::bind (const char *, void *, int)
{
	return (-1);
}

int
AX_New_Allocator::trybind (const char *, void *&)
{
	return (-1);
}

int
AX_New_Allocator::find (const char *, void *&)
{
	return (-1);
}

int
AX_New_Allocator::find (const char *)
{
	return (-1);
}

int
AX_New_Allocator::unbind (const char *)
{
	return (-1);
}

int
AX_New_Allocator::unbind (const char *, void *&)
{
	return (-1);
}

int
AX_New_Allocator::sync (ssize_t, int)
{
	return (-1);
}

int
AX_New_Allocator::sync (void *, size_t, int)
{
	return (-1);
}

int
AX_New_Allocator::protect (ssize_t, int)
{
	return (-1);
}

int
AX_New_Allocator::protect (void *, size_t, int)
{
	return (-1);
}

#if defined (ACE_HAS_MALLOC_STATS)
void
AX_New_Allocator::print_stats (void) const
{
}
#endif /* ACE_HAS_MALLOC_STATS */

void
AX_New_Allocator::dump (void) const
{
#if defined (ACE_HAS_DUMP)
#endif /* ACE_HAS_DUMP */
}

/******************************************************************************/

void *
AX_Static_Allocator_Base::malloc (size_t nbytes)
{
	if (this->offset_ + nbytes > this->size_)
	{
		//printf("ENOMEM \n");
		return 0;
	}
	else
	{
		// Record the current offset, increment the offset by the number
		// of bytes requested, and return the original offset.
		char *ptr = &this->buffer_[this->offset_];
		this->offset_ += nbytes;
		return (void *) ptr;
	}
}

void *
AX_Static_Allocator_Base::calloc (size_t nbytes,
								   char initial_value)
{
	void *ptr = this->malloc (nbytes);

	memset (ptr, initial_value, nbytes);
	return (void *) ptr;
}

void *
AX_Static_Allocator_Base::calloc (size_t n_elem,
								   size_t elem_size,
								   char initial_value)
{
	return this->calloc (n_elem * elem_size, initial_value);
}

void
AX_Static_Allocator_Base::free (void *ptr)
{
	// Check to see if ptr is within our pool?!
	//ACE_UNUSED_ARG (ptr);
	assert (ptr >= this->buffer_ && ptr < this->buffer_ + this->size_);
}

int
AX_Static_Allocator_Base::remove (void)
{
	return -1;
}

int
AX_Static_Allocator_Base::bind (const char *, void *, int)
{
	return -1;
}

int
AX_Static_Allocator_Base::trybind (const char *, void *&)
{
	return -1;
}

int
AX_Static_Allocator_Base::find (const char *, void *&)
{
	return -1;
}

int
AX_Static_Allocator_Base::find (const char *)
{
	return -1;
}

int
AX_Static_Allocator_Base::unbind (const char *)
{
	return -1;
}

int
AX_Static_Allocator_Base::unbind (const char *, void *&)
{
	return -1;
}

int
AX_Static_Allocator_Base::sync (ssize_t, int)
{
	return -1;
}

int
AX_Static_Allocator_Base::sync (void *, size_t, int)
{
	return -1;
}

int
AX_Static_Allocator_Base::protect (ssize_t, int)
{
	return -1;
}

int
AX_Static_Allocator_Base::protect (void *, size_t, int)
{
	return -1;
}

#if defined (ACE_HAS_MALLOC_STATS)
void
ACE_Static_Allocator_Base::print_stats (void) const
{
}
#endif /* ACE_HAS_MALLOC_STATS */

void
AX_Static_Allocator_Base::dump (void) const
{
#if defined (ACE_HAS_DUMP)
	//printf("ACE_Static_Allocator_Base::dump");

	printf ((LM_DEBUG, ACE_BEGIN_DUMP, this));
	printf ((LM_DEBUG, ACE_LIB_TEXT ("\noffset_ = %d"), this->offset_));
	printf ((LM_DEBUG, ACE_LIB_TEXT ("\nsize_ = %d\n"), this->size_));
	//ACE_HEX_DUMP ((LM_DEBUG, this->buffer_, this->size_));
	printf ((LM_DEBUG, ACE_LIB_TEXT ("\n")));

	printf ((LM_DEBUG, ACE_END_DUMP));
#endif /* ACE_HAS_DUMP */
}
