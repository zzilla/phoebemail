template <class AX_LOCK> inline int
AX_Guard<AX_LOCK>::acquire (void)
{
	return this->owner_ = this->lock_->acquire ();
}

template <class AX_LOCK> inline int
AX_Guard<AX_LOCK>::tryacquire (void)
{
	return this->owner_ = this->lock_->tryacquire ();
}

template <class AX_LOCK> inline int
AX_Guard<AX_LOCK>::release (void)
{
	if (this->owner_ == -1)
		return -1;
	else
	{
		this->owner_ = -1;
		return this->lock_->release ();
	}
}

template <class AX_LOCK> inline
AX_Guard<AX_LOCK>::AX_Guard (AX_LOCK &l)
: lock_ (&l),
owner_ (0)
{
	this->acquire ();
}

template <class AX_LOCK> inline
AX_Guard<AX_LOCK>::AX_Guard (AX_LOCK &l, int block)
: lock_ (&l),
owner_ (0)
{
	if (block)
		this->acquire ();
	else
		this->tryacquire ();
}

template <class AX_LOCK> inline
AX_Guard<AX_LOCK>::AX_Guard (AX_LOCK &l, int block, int become_owner)
: lock_ (&l),
owner_ (become_owner == 0 ? -1 : 0)
{
}

// Implicitly and automatically acquire (or try to acquire) the
// lock.

template <class AX_LOCK> inline
AX_Guard<AX_LOCK>::~AX_Guard (void)
{
	this->release ();
}

template <class AX_LOCK> inline int
AX_Guard<AX_LOCK>::locked (void) const
{
	return this->owner_ != -1;
}

template <class AX_LOCK> inline int
AX_Guard<AX_LOCK>::remove (void)
{
	return this->lock_->remove ();
}

template <class AX_LOCK> inline void
AX_Guard<AX_LOCK>::disown (void)
{
	this->owner_ = -1;
}

template <class AX_LOCK> inline
AX_Write_Guard<AX_LOCK>::AX_Write_Guard (AX_LOCK &m)
: AX_Guard<AX_LOCK> (&m)
{
	this->acquire_write ();
}

template <class AX_LOCK> inline int
AX_Write_Guard<AX_LOCK>::acquire_write (void)
{
	return this->owner_ = this->lock_->acquire_write ();
}

template <class AX_LOCK> inline int
AX_Write_Guard<AX_LOCK>::acquire (void)
{
	return this->owner_ = this->lock_->acquire_write ();
}

template <class AX_LOCK> inline int
AX_Write_Guard<AX_LOCK>::tryacquire_write (void)
{
	return this->owner_ = this->lock_->tryacquire_write ();
}

template <class AX_LOCK> inline int
AX_Write_Guard<AX_LOCK>::tryacquire (void)
{
	return this->owner_ = this->lock_->tryacquire_write ();
}

template <class AX_LOCK> inline
AX_Write_Guard<AX_LOCK>::AX_Write_Guard (AX_LOCK &m,
											int block)
											: AX_Guard<AX_LOCK> (&m)
{
	if (block)
		this->acquire_write ();
	else
		this->tryacquire_write ();
}

template <class AX_LOCK> inline int
AX_Read_Guard<AX_LOCK>::acquire_read (void)
{
	return this->owner_ = this->lock_->acquire_read ();
}

template <class AX_LOCK> inline int
AX_Read_Guard<AX_LOCK>::acquire (void)
{
	return this->owner_ = this->lock_->acquire_read ();
}

template <class AX_LOCK> inline int
AX_Read_Guard<AX_LOCK>::tryacquire_read (void)
{
	return this->owner_ = this->lock_->tryacquire_read ();
}

template <class AX_LOCK> inline int
AX_Read_Guard<AX_LOCK>::tryacquire (void)
{
	return this->owner_ = this->lock_->tryacquire_read ();
}

template <class AX_LOCK> inline
AX_Read_Guard<AX_LOCK>::AX_Read_Guard (AX_LOCK &m)
: AX_Guard<AX_LOCK> (&m)
{
	this->acquire_read ();
}

template <class AX_LOCK> inline
AX_Read_Guard<AX_LOCK>::AX_Read_Guard (AX_LOCK &m,
										  int block)
										  : AX_Guard<AX_LOCK> (&m)
{
	if (block)
		this->acquire_read ();
	else
		this->tryacquire_read ();
}

