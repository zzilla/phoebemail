#ifndef AX_THREAD_ADAPTER
#define AX_THREAD_ADAPTER
#include "AX_OS.h"
#include "AX_Thread_Manager.h"
#include "AX_Base_Thread.h"
#include "AX_Thread_Hook.h"
#include "AX_Thread_Exit.h"

class AX_Base_Thread_Adapter
{
public:

	virtual ~AX_Base_Thread_Adapter (void);

	/// Virtual method invoked by the thread entry point.
	virtual AX_thr_func_return invoke (void);

	/// Accessor for the C entry point function to the OS thread creation
	/// routine.
	AX_THR_C_FUNC entry_point (void);

protected:
	/// Constructor.
	AX_Base_Thread_Adapter ( AX_THR_FUNC user_func,
		void *arg,
		AX_THR_C_FUNC entry_point ,
		AX_OS_Thread_Descriptor *td = 0
# ifdef _WIN32
		, AX_SEH_EXCEPT_HANDLER selector = 0
		, AX_SEH_EXCEPT_HANDLER handler = 0
# endif /* ACE_HAS_WIN32_STRUCTURAL_EXCEPTIONS */
		);

private:

protected:
	/// Thread startup function passed in by the user (C++ linkage).
	AX_THR_FUNC user_func_;

	/// Argument to thread startup function.
	void *arg_;

	/// Entry point to the underlying OS thread creation call (C
	/// linkage).
	AX_THR_C_FUNC entry_point_;

	/**
	* Optional thread descriptor.  Passing this pointer in will force
	* the spawned thread to cache this location in <Log_Msg> and wait
	* until <Thread_Manager> fills in all information in thread
	* descriptor.
	*/
	AX_OS_Thread_Descriptor *thr_desc_;
#ifdef ACE_USES_GPROF
	struct timerval itimer_;
#endif // ACE_USES_GPROF


};
class AX_Thread_Adapter : public AX_Base_Thread_Adapter
{
public:
	/// Constructor.
	AX_Thread_Adapter( AX_THR_FUNC user_func,		
		void *arg,	
		AX_THR_C_FUNC entry_point =NULL,
		//AX_Thread_Manager *thr_mgr = 0,
		AX_Thread_Descriptor *td = 0,
		AX_Thread_Exit *exit  = 0,
		AX_Thread_Hook *hook = 0
# if defined (_WIN32)
		,AX_SEH_EXCEPT_HANDLER selector = 0
		,AX_SEH_EXCEPT_HANDLER handler = 0
# endif /* ACE_HAS_WIN32_STRUCTURAL_EXCEPTIONS */
		);
	AX_Thread_Adapter( AX_THR_FUNC user_func,		
		void *arg,	
		AX_THR_C_FUNC entry_point =NULL,
		//AX_Thread_Manager *thr_mgr = 0,
		AX_Thread_Descriptor *td = 0
# if defined (_WIN32)
		,AX_SEH_EXCEPT_HANDLER selector = 0
		,AX_SEH_EXCEPT_HANDLER handler = 0
# endif /* ACE_HAS_WIN32_STRUCTURAL_EXCEPTIONS */
		);

	/**
	* Execute the <user_func_> with the <arg>.  This function deletes
	* <this>, thereby rendering the object useless after the call
	* returns.
	*/
	virtual AX_thr_func_return invoke (void);

	/// Accessor for the optional <Thread_Manager>.
	//AX_Thread_Manager *thr_mgr (void);

protected:

	/// Ensure that this object must be allocated on the heap.
	~AX_Thread_Adapter (void);

private:

	/// Called by invoke, mainly here to separate the SEH stuff because
	/// SEH on Win32 doesn't compile with local vars with destructors.
	virtual AX_thr_func_return invoke_i (void);

private:

	/// Optional thread manager.
	//AX_Thread_Manager *thr_mgr_;
	//////////////////////////////////////////6ÔÂ27ÈÕ
	AX_Thread_Exit *exit_;
	AX_Thread_Hook *hook_;

};
#include "AX_Thread_Adapter.inl"
#endif
