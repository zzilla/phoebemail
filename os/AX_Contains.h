#ifndef _AX_CONTAINS_H
#define _AX_CONTAINS_H

#include "AX_OS.h"
#include "AX_Contains.h"
#include "AX_Malloc.h"
template <class T> class AX_Double_Linked_List_Iterator;
template <class T> class AX_Double_Linked_List ;

template <class T>
class AX_Double_Linked_List_Iterator_Base
{
public:
	// = Iteration methods.

	/// Passes back the {entry} under the iterator. Returns 0 if the
	/// iteration has completed, otherwise 1
	int next (T *&) const;

	/**
	* @deprecated Return the address of next (current) unvisited item in
	* the list. 0 if there is no more element available.
	*/
	T *next (void) const;

	/// Returns 1 when all items have been seen, else 0.
	int done (void) const;

	/// STL-like iterator dereference operator: returns a reference
	/// to the node underneath the iterator.
	T & operator* (void) const ;

	/**
	* Retasks the iterator to iterate over a new
	* Double_Linked_List. This allows clients to reuse an iterator
	* without incurring the constructor overhead. If you do use this,
	* be aware that if there are more than one reference to this
	* iterator, the other "clients" may be very bothered when their
	* iterator changes.  @@ Here be dragons. Comments?
	*/
	void reset (AX_Double_Linked_List<T> &);

protected:
	// = Initialization methods.

	/// Constructor
	AX_Double_Linked_List_Iterator_Base (const AX_Double_Linked_List<T>  &);

	/// Copy constructor.
	AX_Double_Linked_List_Iterator_Base (const
		AX_Double_Linked_List_Iterator_Base<T>
		&iter);

	// = Iteration methods.
	/**
	* Move to the first element of the list. Returns 0 if the list is
	* empty, else 1.
	* @note the head of the ACE_DLList is actually a null entry, so the
	* first element is actually the 2n'd entry
	*/
	int go_head (void);

	/// Move to the last element of the list. Returns 0 if the list is
	/// empty, else 1.
	int go_tail (void);

	/**
	* Check if we reach the end of the list.  Can also be used to get
	* the *current* element in the list.  Return the address of the
	* current item if there are still elements left , 0 if we run out
	* of element.
	*/
	T *not_done (void) const ;

	/// Advance to the next element in the list.  Return the address of the
	/// next element if there are more, 0 otherwise.
	T *do_advance (void);

	/// Retreat to the previous element in the list.  Return the address
	/// of the previous element if there are more, 0 otherwise.
	T *do_retreat (void);

	/// Dump the state of an object.
	void dump_i (void) const;

	/// Remember where we are.
	T *current_;

	const AX_Double_Linked_List<T>  *dllist_;
};
template <class T>
class AX_Double_Linked_List_Iterator : public AX_Double_Linked_List_Iterator_Base <T>
{
public:
	// = Initialization method.
	AX_Double_Linked_List_Iterator (const AX_Double_Linked_List<T> &);

	/**
	* Retasks the iterator to iterate over a new
	* Double_Linked_List. This allows clients to reuse an iterator
	* without incurring the constructor overhead. If you do use this,
	* be aware that if there are more than one reference to this
	* iterator, the other "clients" may be very bothered when their
	* iterator changes.
	* @@ Here be dragons. Comments?
	*/
	void reset (AX_Double_Linked_List<T> &);

	/// Move to the first element in the list.  Returns 0 if the
	/// list is empty, else 1.
	int first (void);

	/// Move forward by one element in the list.  Returns 0 when all the
	/// items in the list have been seen, else 1.
	int advance (void);

	/**
	* Advance the iterator while removing the original item from the
	* list.  Return a pointer points to the original (removed) item.
	* If {dont_remove} equals 0, this function behaves like {advance}
	* but return 0 (NULL) instead.
	*/
	T* advance_and_remove (int dont_remove);

	// = STL-style iteration methods

	/// Prefix advance.
	AX_Double_Linked_List_Iterator<T> & operator++ (void);

	/// Postfix advance.
	AX_Double_Linked_List_Iterator<T> operator++ (int);

	/// Prefix reverse.
	AX_Double_Linked_List_Iterator<T> & operator-- (void);

	/// Postfix reverse.
	AX_Double_Linked_List_Iterator<T> operator-- (int);

	/// Dump the state of an object.
	void dump (void) const;

};
template <class T>
class AX_Double_Linked_List
{
public:
	friend class AX_Double_Linked_List_Iterator_Base<T>;
	friend class AX_Double_Linked_List_Iterator<T>;
	//friend class AX_Double_Linked_List_Reverse_Iterator<T>;

	// Trait definition.
	typedef AX_Double_Linked_List_Iterator<T> ITERATOR;
	//typedef AX_Double_Linked_List_Reverse_Iterator<T> REVERSE_ITERATOR;

	// = Initialization and termination methods.
	/// construction.  Use user specified allocation strategy
	/// if specified.
	/**
	* Initialize an empy list using the allocation strategy specified by the user.
	* If none is specified, then use default allocation strategy.
	*/
	AX_Double_Linked_List (AX_Allocator *the_allocator = 0);

	/// Copy constructor.
	/**
	* Create a double linked list that is a copy of the provided
	* parameter.
	*/
	AX_Double_Linked_List (const AX_Double_Linked_List<T> &);

	/// Assignment operator.
	/**
	* Perform a deep copy of the provided list by first deleting the nodes of the
	* lhs and then copying the nodes of the rhs.
	*/
	void operator= (const AX_Double_Linked_List<T> &);

	/// Destructor.
	/**
	* Clean up the memory allocated for the nodes of the list.
	*/
	~AX_Double_Linked_List (void);

	// = Check boundary conditions.

	/// Returns 1 if the container is empty, 0 otherwise.
	/**
	* Performs constant time check to determine if the list is empty.
	*/
	int is_empty (void) const;

	/// The list is unbounded, so this always returns 0.
	/**
	* Since the list is unbounded, the method simply returns 0.
	*/
	int is_full (void) const;

	// = Classic queue operations.

	/// Adds <new_item> to the tail of the list. Returns the new item
	/// that was inserted.
	/**
	* Provides constant time insertion at the end of the list structure.
	*/
	T *insert_tail (T *new_item);

	/// Adds <new_item> to the head of the list.Returns the new item that
	/// was inserted.
	/**
	* Provides constant time insertion at the head of the list.
	*/
	T *insert_head (T *new_item);

	/// Removes the head of the list and returns a pointer to that item.
	/**
	* Removes and returns the first {item} in the list.  Returns
	* internal node's address on success, 0 if the queue was empty.
	* This method will *not* free the internal node.
	*/
	T* delete_head (void);
	/////////////////////////  6/26
	T* dequeue_head (T* td);
	T* add (T* new_item);
	T *enqueue_tail (T *new_item);
	T* remove (void);

	/// Removes the tail of the list and returns a pointer to that item.
	/**
	* Removes and returns the last {item} in the list.  Returns
	* internal nodes's address on success, 0 if the queue was
	* empty. This method will *not* free the internal node.
	*/
	T *delete_tail (void);

	// = Additional utility methods.

	///Empty the list.
	/**
	* Reset the {ACE_Double_Linked_List} to be empty.
	* Notice that since no one is interested in the items within,
	* This operation will delete all items.
	*/
	void reset (void);

	/// Get the {slot}th element in the set.  Returns -1 if the element
	/// isn't in the range {0..{size} - 1}, else 0.
	/**
	* Iterates through the list to the desired index and assigns the provides pointer
	* with the address of the node occupying that index.
	*/
	int get (T *&item, size_t slot = 0);

	/// The number of items in the queue.
	/**
	* Constant time call to return the current size of the list.
	*/
	size_t size (void) const;

	/// Dump the state of an object.
	void dump (void) const;

	/// Use DNode address directly.
	/**
	* Constant time removal of an item from the list using it's address.
	*/
	int remove (T *n);

protected:
	/// Delete all the nodes in the list.
	/**
	* Removes and deallocates memory for all of the list nodes.
	*/
	void delete_nodes (void);

	/// Copy nodes from {rhs} into this list.
	/**
	* Copy the elements of the provided list by allocated new nodes and assigning
	* them with the proper data.
	*/
	void copy_nodes (const AX_Double_Linked_List<T> &rhs);

	/// Setup header pointer.  Called after we create the head node in ctor.
	/**
	* Initialize the head pointer so that the list has a dummy node.
	*/
	void init_head (void);

	///Constant time insert a new item into the list structure.
	/**
	* Insert a @a new_item into the list.  It will be added before
	* or after @a old_item.  Default is to insert the new item *after*
	* {head_}.  Return 0 if succeed, -1 if error occured.
	*/
	int insert_element (T *new_item,
		int before = 0,
		T *old_item = 0);

	///Constant time delete an item from the list structure.
	/**
	* Remove @a item from the list.  Return 0 if succeed, -1 otherwise.
	* Notice that this function checks if item is {head_} and either its
	* {next_} or {prev_} is NULL.  The function resets item's {next_} and
	* {prev_} to 0 to prevent clobbering the double-linked list if a user
	* tries to remove the same node again.
	*/
	int remove_element (T *item);

	/// Head of the circular double-linked list.
	T *head_;

	/// Size of this list.
	size_t size_;

	/// Allocation Strategy of the queue.
	AX_Allocator *allocator_;
};
#endif
