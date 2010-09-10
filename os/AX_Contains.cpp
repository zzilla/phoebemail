#include "AX_Contains.h"

//--------------------------------------------------
//ACE_ALLOC_HOOK_DEFINE(AX_Double_Linked_List_Iterator_Base)

template <class T>
AX_Double_Linked_List_Iterator_Base<T>::AX_Double_Linked_List_Iterator_Base (const AX_Double_Linked_List<T> &dll)
: current_ (0), dllist_ (&dll)
{
	// Do nothing
}

template <class T>
AX_Double_Linked_List_Iterator_Base<T>::AX_Double_Linked_List_Iterator_Base (const AX_Double_Linked_List_Iterator_Base<T> &iter)
: current_ (iter.current_),
dllist_ (iter.dllist_)
{
	// Do nothing
}


template <class T> T *
AX_Double_Linked_List_Iterator_Base<T>::next (void) const
{
	return this->not_done ();
}

template <class T> int
AX_Double_Linked_List_Iterator_Base<T>::next (T *&ptr) const
{
	ptr = this->not_done ();
	return ptr ? 1 : 0;
}


template <class T> int
AX_Double_Linked_List_Iterator_Base<T>::done (void) const
{
	return this->not_done () ? 0 : 1;
}

template <class T> T &
AX_Double_Linked_List_Iterator_Base<T>::operator* (void) const
{
	return *(this->not_done ());
}

// @@ Is this a valid retasking? Make sure to check with Purify and
// whatnot that we're not leaking memory or doing any other screwing things.
template <class T> void
AX_Double_Linked_List_Iterator_Base<T>::reset (AX_Double_Linked_List<T> &dll)
{
	current_ = 0;
	dllist_ = &dll;
}

template <class T> int
AX_Double_Linked_List_Iterator_Base<T>::go_head (void)
{
	this->current_ = static_cast<T*> (dllist_->head_->next_);
	return this->current_ ? 1 : 0;
}

template <class T> int
AX_Double_Linked_List_Iterator_Base<T>::go_tail (void)
{
	this->current_ = static_cast<T*> (dllist_->head_->prev_);
	return this->current_ ? 1 : 0;
}

template <class T> T *
AX_Double_Linked_List_Iterator_Base<T>::not_done (void) const
{
	if (this->current_ != this->dllist_->head_)
		return this->current_;
	else
		return 0;
}

template <class T> T *
AX_Double_Linked_List_Iterator_Base<T>::do_advance (void)
{
	if (this->not_done ())
	{
		this->current_ = static_cast<T*> (this->current_->next_);
		return this->not_done ();
	}
	else
		return 0;
}

template <class T> T *
AX_Double_Linked_List_Iterator_Base<T>::do_retreat (void)
{
	if (this->not_done ())
	{
		this->current_ = static_cast<T*> (this->current_->prev_);
		return this->not_done ();
	}
	else
		return 0;
}

template <class T> void
AX_Double_Linked_List_Iterator_Base<T>::dump_i (void) const
{
	//printf ("current_ = %x"), this->current_);
	//ACE_DEBUG ((LM_DEBUG, ACE_END_DUMP));
}

//--------------------------------------------------
//ACE_ALLOC_HOOK_DEFINE(AX_Double_Linked_List_Iterator)

template <class T>
AX_Double_Linked_List_Iterator<T>::AX_Double_Linked_List_Iterator (const AX_Double_Linked_List<T> &dll)
: AX_Double_Linked_List_Iterator_Base <T> (dll)
{
	this->current_ = static_cast<T*> (dll.head_->next_);
	// Advance current_ out of the null area and onto the first item in
	// the list
}

template <class T> void
AX_Double_Linked_List_Iterator<T>::reset (AX_Double_Linked_List<T> &dll)
{
	this->AX_Double_Linked_List_Iterator_Base <T>::reset (dll);
	this->current_ = static_cast<T*> (dll.head_->next_);
	// Advance current_ out of the null area and onto the first item in
	// the list
}

template <class T> int
AX_Double_Linked_List_Iterator<T>::first (void)
{
	return this->go_head ();
}

template <class T> int
AX_Double_Linked_List_Iterator<T>::advance (void)
{
	return this->do_advance () ? 1 : 0;
}

template <class T> T*
AX_Double_Linked_List_Iterator<T>::advance_and_remove (int dont_remove)
{
	T* item = 0;
	if (dont_remove)
		this->do_advance ();
	else
	{
		item = this->next ();
		this->do_advance ();
		// It seems dangerous to remove nodes in an iterator, but so it goes...
		AX_Double_Linked_List<T> *dllist =
			const_cast<AX_Double_Linked_List<T> *> (this->dllist_);
		dllist->remove (item);
	}
	return item;
}

template <class T> void
AX_Double_Linked_List_Iterator<T>::dump (void) const
{
#if defined (ACE_HAS_DUMP)
	this->dump_i ();
#endif /* ACE_HAS_DUMP */
}

// Prefix advance.

template <class T>
AX_Double_Linked_List_Iterator<T> &
AX_Double_Linked_List_Iterator<T>::operator++ (void)
{
	this->do_advance ();
	return *this;
}


// Postfix advance.

template <class T>
AX_Double_Linked_List_Iterator<T>
AX_Double_Linked_List_Iterator<T>::operator++ (int)
{
	AX_Double_Linked_List_Iterator<T> retv (*this);
	this->do_advance ();
	return retv;
}


// Prefix reverse.

template <class T>
AX_Double_Linked_List_Iterator<T> &
AX_Double_Linked_List_Iterator<T>::operator-- (void)
{
	this->do_retreat ();
	return *this;
}


// Postfix reverse.

template <class T>
AX_Double_Linked_List_Iterator<T>
AX_Double_Linked_List_Iterator<T>::operator-- (int)
{
	AX_Double_Linked_List_Iterator<T> retv (*this);
	this->do_retreat ();
	return retv;
}


//--------------------------------------------------
//ACE_ALLOC_HOOK_DEFINE(AX_Double_Linked_List_Reverse_Iterator)
//
//template <class T>
//ACE_Double_Linked_List_Reverse_Iterator<T>::ACE_Double_Linked_List_Reverse_Iterator (ACE_Double_Linked_List<T> &dll)
//: ACE_Double_Linked_List_Iterator_Base <T> (dll)
//{
//	this->current_ = static_cast<T*> (dll.head_->prev_);
//	// Advance current_ out of the null area and onto the last item in
//	// the list
//}
//
//template <class T> void
//ACE_Double_Linked_List_Reverse_Iterator<T>::reset (ACE_Double_Linked_List<T> &dll)
//{
//	this->ACE_Double_Linked_List_Iterator_Base <T>::reset (dll);
//	this->current_ = static_cast<T*> (dll.head_->prev_);
//	// Advance current_ out of the null area and onto the last item in
//	// the list
//}
//
//template <class T> int
//ACE_Double_Linked_List_Reverse_Iterator<T>::first (void)
//{
//	return this->go_tail ();
//}
//
//template <class T> int
//ACE_Double_Linked_List_Reverse_Iterator<T>::advance (void)
//{
//	return this->do_retreat () ? 1 : 0;
//}
//
//template <class T> T*
//ACE_Double_Linked_List_Reverse_Iterator<T>::advance_and_remove (int dont_remove)
//{
//	T* item = 0;
//	if (dont_remove)
//		this->do_retreat ();
//	else
//	{
//		item = this->next ();
//		this->do_retreat ();
//		// It seems dangerous to remove nodes in an iterator, but so it goes...
//		ACE_Double_Linked_List<T> *dllist =
//			const_cast<ACE_Double_Linked_List<T> *> (this->dllist_);
//		dllist->remove (item);
//	}
//	return item;
//}
//
//template <class T> void
//ACE_Double_Linked_List_Reverse_Iterator<T>::dump (void) const
//{
//#if defined (ACE_HAS_DUMP)
//	this->dump_i ();
//#endif /* ACE_HAS_DUMP */
//}
//
//// Prefix advance.
//
//template <class T>
//ACE_Double_Linked_List_Reverse_Iterator<T> &
//ACE_Double_Linked_List_Reverse_Iterator<T>::operator++ (void)
//{
//	this->do_retreat ();
//	return *this;
//}
//
//
//// Postfix advance.
//
//template <class T>
//ACE_Double_Linked_List_Reverse_Iterator<T>
//ACE_Double_Linked_List_Reverse_Iterator<T>::operator++ (int)
//{
//	ACE_Double_Linked_List_Reverse_Iterator<T> retv (*this);
//	this->do_retreat ();
//	return retv;
//}
//
//
//// Prefix reverse.
//
//template <class T>
//ACE_Double_Linked_List_Reverse_Iterator<T> &
//ACE_Double_Linked_List_Reverse_Iterator<T>::operator-- (void)
//{
//	this->do_advance ();
//	return *this;
//}
//
//
//// Postfix reverse.
//
//template <class T>
//ACE_Double_Linked_List_Reverse_Iterator<T>
//ACE_Double_Linked_List_Reverse_Iterator<T>::operator-- (int)
//{
//	ACE_Double_Linked_List_Reverse_Iterator<T> retv (*this);
//	this->do_advance ();
//	return retv;
//}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//ACE_ALLOC_HOOK_DEFINE(AX_Double_Linked_List)

template <class T>
AX_Double_Linked_List<T>:: AX_Double_Linked_List (AX_Allocator *alloc)
: size_ (0), allocator_ (alloc)
{
	if (this->allocator_ == 0)
		this->allocator_ = AX_Allocator::instance ();

	AX_NEW_MALLOC (this->head_,
		(T *) this->allocator_->malloc (sizeof (T)),T);
	this->init_head ();
}

template <class T>
AX_Double_Linked_List<T>::AX_Double_Linked_List (const AX_Double_Linked_List<T> &cx)
: allocator_ (cx.allocator_)
{
	if (this->allocator_ == 0)
		this->allocator_ = AX_Allocator::instance ();

	AX_NEW_MALLOC (this->head_,
		(T *) this->allocator_->malloc (sizeof (T)),
		T);
	this->init_head ();
	this->copy_nodes (cx);
	this->size_ = cx.size_;
}

template <class T> void
AX_Double_Linked_List<T>::operator= (const AX_Double_Linked_List<T> &cx)
{
	if (this != &cx)
	{
		this->delete_nodes ();
		this->copy_nodes (cx);
	}
}

template <class T>
AX_Double_Linked_List<T>::~AX_Double_Linked_List (void)
{
	this->delete_nodes ();

	AX_DES_FREE (head_,
		this->allocator_->free,
		T);

	this->head_ = 0;
}

template <class T> int
AX_Double_Linked_List<T>::is_empty (void) const
{
	return this->size () ? 0 : 1;
}

template <class T> int
AX_Double_Linked_List<T>::is_full (void) const
{
	return 0;                     // We have no bound.
}

template <class T> T *
AX_Double_Linked_List<T>::insert_tail (T *new_item)
{
	// Insert it before <head_>, i.e., at tail.
	this->insert_element (new_item, 1);
	return new_item;
}

template <class T> T *
AX_Double_Linked_List<T>::insert_head (T *new_item)
{
	this->insert_element (new_item); // Insert it after <head_>, i.e., at head.
	return new_item;
}

template <class T> T *
AX_Double_Linked_List<T>::delete_head (void)
{
	T *temp;

	if (this->is_empty ())
		return 0;

	temp = static_cast<T *> (this->head_->next_);
	// Detach it from the list.
	this->remove_element (temp);
	return temp;
}

template <class T> T *
AX_Double_Linked_List<T>::delete_tail (void)
{
	T *temp;

	if (this->is_empty ())
		return 0;

	temp = static_cast <T *> (this->head_->prev_);
	// Detach it from the list.
	this->remove_element (temp);
	return temp;
}

template <class T> void
AX_Double_Linked_List<T>::reset (void)
{
	this->delete_nodes ();
}

template <class T> int
AX_Double_Linked_List<T>::get (T *&item, size_t slot)
{
	AX_Double_Linked_List_Iterator<T> iter (*this);

	for (size_t i = 0;
		i < slot && !iter.done ();
		i++)
		iter.advance ();

	item = iter.next ();
	return item ? 0 : -1;
}

template <class T> size_t
AX_Double_Linked_List<T>::size (void) const
{
	return this->size_;
}

template <class T> void
AX_Double_Linked_List<T>::dump (void) const
{
#if defined (ACE_HAS_DUMP)
	// Dump the state of an object.
#endif /* ACE_HAS_DUMP */
}

#if 0
template <class T> T *
ACE_Double_Linked_List<T>::find (const T &item)
{
	for (ACE_Double_Linked_List_Iterator<T> iter (*this);
		!iter.done ();
		iter.advance ())
	{
		T *temp = iter.next ();

		if (*temp == item)
			return temp;
	}

	return 0;
}

template <class T> int
ACE_Double_Linked_List<T>::remove (const T &item)
{
	T *temp = this->find (item);

	if (temp != 0)
		return this->remove (temp);
	else
		return -1;
}
#endif /* 0 */

template <class T> int
AX_Double_Linked_List<T>::remove (T *n)
{
	return this->remove_element (n);
}

template <class T> void
AX_Double_Linked_List<T>::delete_nodes (void)
{
	while (! this->is_empty ())
	{
		T * temp = static_cast<T*> (this->head_->next_);
		this->remove_element (temp);
		AX_DES_FREE (temp,
			this->allocator_->free,
			T);
	}
}

template <class T> void
AX_Double_Linked_List<T>::copy_nodes (const AX_Double_Linked_List<T> &c)
{
	for (AX_Double_Linked_List_Iterator<T> iter (c);
		!iter.done ();
		iter.advance ())
	{
		T* temp = 0;
		AX_NEW_MALLOC (temp,
			(T *)this->allocator_->malloc (sizeof (T)),
			T (*iter.next ()));
		this->insert_tail (temp);
	}
}

template <class T> void
AX_Double_Linked_List<T>::init_head (void)
{
	this->head_->next_ = this->head_;
	this->head_->prev_ = this->head_;
}

template <class T> int
AX_Double_Linked_List<T>::insert_element (T *new_item,
										   int before,
										   T *old_item)
{
	if (old_item == 0)
		old_item = this->head_;

	if (before)
		old_item = static_cast<T *> (old_item->prev_);

	new_item->next_ = old_item->next_;
	new_item->next_->prev_ = new_item;
	new_item->prev_ = old_item;
	old_item->next_ = new_item;
	this->size_++;
	return 0;                     // Well, what will cause errors here?
}

template <class T> int
AX_Double_Linked_List<T>::remove_element (T *item)
{
	// Notice that you have to ensure that item is an element of this
	// list.  We can't do much checking here.

	if (item == this->head_ || item->next_ == 0
		|| item->prev_ == 0 || this->size () == 0)      // Can't remove head
		return -1;

	item->prev_->next_ = item->next_;
	item->next_->prev_ = item->prev_;
	item->next_ = item->prev_ = 0; // reset pointers to prevent double removal.
	this->size_--;
	return 0;
}
template<class T> T*
AX_Double_Linked_List<T>::dequeue_head(T* td)
{
	return (T*)NULL;
}
