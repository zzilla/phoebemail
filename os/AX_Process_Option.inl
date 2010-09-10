
 AX_HANDLE
AX_Process::gethandle (void) const
{
#ifdef _WIN32
	return process_info_.hProcess;
#else
	return AX_HANDLE (child_id_);
#endif /* ACE_WIN32 */
}
inline pid_t
AX_Process::getpid (void)
const
{
#ifdef _WIN32
	return process_info_.dwProcessId;
#else /* ACE_WIN32 */
	return child_id_;
#endif /* ACE_WIN32 */
}
inline pid_t
AX_Process::wait (int *status,
				   int wait_options)
{
	pid_t retv =
		AX_OS::wait (this->getpid (),
		&this->exit_code_,
		wait_options
#if defined (_WIN32)
		, process_info_.hProcess
#endif /* ACE_WIN32 */
		);
	if (status != 0)
		*status = this->exit_code_;

	return retv;
}
inline int
AX_Process::kill (int signum)
{
	if (this->getpid () != -1)
		return AX_OS::kill (this->getpid (), signum);
	else
		return -1;
}

inline int
AX_Process::terminate (void)
{
	if (this->getpid () != -1)
		return ACE::terminate_process (this->getpid ());
		return 
	else
		return -1;
}
inline int
AX_Process::return_value (void) const
{
	return exit_code_;
}

inline int
AX_Process::exit_code (void) const
{
	return this->exit_code_;
}
inline void
AX_Process::exit_code (int code)
{
	this->exit_code_ = code;
}

inline void
AX_Process_Options::creation_flags (u_long cf)
{
	creation_flags_ = cf;
}
inline pid_t
AX_Process_Options::getgroup (void) const
{
	return process_group_;
}

inline pid_t
AX_Process_Options::setgroup (pid_t pgrp)
{
	pid_t old = process_group_;
	process_group_ = pgrp;
	return old;
}
inline int
AX_Process_Options::handle_inheritence (void)
{
	return handle_inheritence_;
}
inline void
AX_Process_Options::handle_inheritence (int hi)
{
	handle_inheritence_ = hi;
}

inline int
AX_Process_Options::avoid_zombies (void)
{
	return avoid_zombies_;
}
inline void
AX_Process_Options::avoid_zombies (int avoid_zombies)
{
	avoid_zombies_ = avoid_zombies;
}

#if defined (_WIN32)

inline STARTUPINFO *
AX_Process_Options::startup_info (void)
{
	return &startup_info_;
}

inline LPSECURITY_ATTRIBUTES
AX_Process_Options::get_process_attributes (void) const
{
#if defined (_WIN32)
	return process_attributes_;
#else
	return 0;
#endif /* !ACE_HAS_WINCE */
}

inline LPSECURITY_ATTRIBUTES
AX_Process_Options::set_process_attributes (void)
{
	process_attributes_ = &security_buf1_;
	return process_attributes_;
}

inline LPSECURITY_ATTRIBUTES
AX_Process_Options::get_thread_attributes (void) const
{
	return thread_attributes_;
}

inline LPSECURITY_ATTRIBUTES
AX_Process_Options::set_thread_attributes (void)
{
	thread_attributes_ = &security_buf2_;
	return thread_attributes_;
}

#else /* !defined (ACE_WIN32) */

inline AX_HANDLE
AX_Process_Options::get_stdin (void) const
{
	return stdin_;
}

inline AX_HANDLE
AX_Process_Options::get_stdout (void) const
{
	return stdout_;
}

inline AX_HANDLE
AX_Process_Options::get_stderr (void) const
{
	return stderr_;
}

inline int
AX_Process_Options::inherit_environment (void) const
{
	return inherit_environment_;
}

inline void
AX_Process_Options::inherit_environment (int nv)
{
	inherit_environment_ = nv;
}

inline int
AX_Process_Options::setreugid (const char* user)
{
#if !defined (ACE_LACKS_PWD_FUNCTIONS)
	struct passwd *ent = ACE_OS::getpwnam (user);

	if (ent != 0)
	{
		this->euid_ = ent->pw_uid;
		this->ruid_ = ent->pw_uid;
		this->egid_ = ent->pw_gid;
		this->rgid_ = ent->pw_gid;
		return 0;
	}
	else
		return -1;
#else
	ACE_UNUSED_ARG (user);
	ACE_NOTSUP_RETURN (-1);
#endif /* ACE_LACKS_PWD_FUNCTIONS */
}

inline void
AX_Process_Options::setruid (uid_t id)
{
	this->ruid_ = id;
}

inline void
AX_Process_Options::seteuid (uid_t id)
{
	this->euid_ = id;
}

inline void
AX_Process_Options::setrgid (uid_t id)
{
	this->rgid_ = id;
}

inline void
AX_Process_Options::setegid (uid_t id)
{
	this->egid_ = id;
}

inline uid_t
AX_Process_Options::getruid (void) const
{
	return this->ruid_;
}

inline uid_t
AX_Process_Options::geteuid (void) const
{
	return this->euid_;
}

inline uid_t
AX_Process_Options::getrgid (void) const
{
	return this->rgid_;
}

inline uid_t
AX_Process_Options::getegid (void) const
{
	return this->egid_;
}
#endif /* ACE_WIN32 */

inline char *
AX_Process_Options::command_line_buf (int *max_lenp)
{
	if (max_lenp != 0)
		*max_lenp = this->command_line_buf_len_;
	return this->command_line_buf_;
}

inline char *
AX_Process_Options::working_directory (void)
{
	if (working_directory_[0] == '\0')
		return 0;
	else
		return working_directory_;
}

inline void
AX_Process_Options::working_directory (const char *wd)
{
	strcpy (working_directory_, wd);
}

inline void
AX_Process_Options::working_directory (const char *wd)
{
	strcpy (working_directory_, wd);
}

inline void
AX_Process_Options::process_name (const char *p)
{
	strcpy (this->process_name_, p);
}

inline const char *
AX_Process_Options::process_name (void)
{
	if (process_name_[0] == '\0')
		this->process_name (this->command_line_argv ()[0]);

	return this->process_name_;
}

//#if defined (ACE_HAS_WINCE)
// Here is a collection of inline functions which are defined only
// under CE.  They are not empty on most other platforms.

inline int
AX_Process_Options::setenv (char * /* envp */[])
{
	return -1;
}

inline int
AX_Process_Options::setenv (const char * /* format */, ...)
{
	return -1;
}

inline int
AX_Process_Options::setenv (const char * /* variable_name */,
							 const char * /* format */,
							 ...)
{
	return -1;
}

inline int
AX_Process_Options::set_handles (AX_HANDLE /* std_in */,
								  AX_HANDLE /* std_out */,
								  AX_HANDLE /* std_err */)
{
	return -1;
}

