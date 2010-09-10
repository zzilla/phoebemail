#ifndef _AX_PROCESS_OPTION
#define _AX_PROCESS_OPTION
#include "AX_OS.h"
class  AX_Process_Options
{
public:
	enum
	{
		DEFAULT_COMMAND_LINE_BUF_LEN = 1024,
		// UNIX process creation flags.
#if defined (_WIN32)
		NO_EXEC = 0
#else
		NO_EXEC = 1
#endif /* ACE_WIN32 */
	};

protected:
	// = Default settings not part of public Interface.
	//
	// @@todo These sizes should be taken from the appropriate
	// POSIX/system header files and/or defined dynamically.
	enum
	{
		MAX_COMMAND_LINE_OPTIONS = 128,
		ENVIRONMENT_BUFFER = 16 * 1024, // 16K
		MAX_ENVIRONMENT_ARGS = 512 //
	};

public:
	/**
	* If @a inherit_environment == 1, the new process will inherit the
	* environment of the current process.  @a command_line_buf_len is the
	* max strlen for command-line arguments.
	*/
	AX_Process_Options (int inherit_environment = 1,
		int command_line_buf_len = DEFAULT_COMMAND_LINE_BUF_LEN,
		int env_buf_len = ENVIRONMENT_BUFFER,
		int max_env_args = MAX_ENVIRONMENT_ARGS);

	/// Destructor.
	~AX_Process_Options (void);

	// = Methods to set process creation options portably.

	/**
	* Set the standard handles of the new process to the respective
	* handles.  If you want to affect a subset of the handles, make
	* sure to set the others to ACE_INVALID_HANDLE.  Returns 0 on
	* success, -1 on failure.
	*/
	int set_handles (AX_HANDLE std_in,
		AX_HANDLE std_out = AX_INVALID_HANDLE,
		AX_HANDLE std_err = AX_INVALID_HANDLE);

	/// Release the standard handles previously set with set_handles;
	void release_handles (void);

	/// @param format must be of the form "VARIABLE=VALUE".  There can not be
	/// any spaces between VARIABLE and the equal sign.
	int setenv (const char *format,
		...);

	/**
	* Set a single environment variable, @a variable_name.  Since
	* different platforms separate each environment variable
	* differently, you must call this method once for each variable.
	* <format> can be any printf format string.  So options->setenv
	* ("FOO","one + two = %s", "three") will result in "FOO=one + two =
	* three".
	*/
	int setenv (const char *variable_name,
		const char *format,
		...);

	/// Same as above with argv format.  @a envp must be null terminated.
	int setenv (char *envp[]);

	/// Set the working directory for the process.  strlen of @a wd must
	/// be <= MAXPATHLEN.
	void working_directory (const char *wd);
//
//#if defined (ACE_HAS_WCHAR)
//	/// wchar_t version of working_directory
//	void working_directory (const wchar_t *wd);
//#endif /* ACE_HAS_WCHAR */

	/**
	* Set the command-line arguments.  @a format can use any printf
	* formats.  The first token in @a format should be the path to the
	* application.  This can either be a full path, relative path, or
	* just an executable name.  If an executable name is used, we rely
	* on the platform's support for searching paths.  Since we need a
	* path to run a process, this method *must* be called!  Returns 0
	* on success, -1 on failure.
	*/
	int command_line (const char *format, ...);

//#if defined (ACE_HAS_WCHAR) && !defined (ACE_HAS_WINCE)
//	/// Anti-TChar version of command_line ()
//	int command_line (const ACE_ANTI_TCHAR *format, ...);
//#endif /* ACE_HAS_WCHAR && !ACE_HAS_WINCE */

	/// Same as above in argv format.  @a argv must be null terminated.
	int command_line (const char * const argv[]);

	// = Set/get the pathname used to name the process.
	/**
	* Specify the full path or relative path, or just the executable
	* name for the process. If this is set, then @a name will be used to
	* create the process instead of argv[0] set in the command
	* line. This is here so that you can supply something other than
	* executable name as argv[0].
	*/
	void process_name (const char *name);

	/// Return the process_name.  If the <process_name(name)> set
	/// method is not called, this method will return argv[0].
	const char *process_name (void);

	/// Get the creation flags.
	u_long creation_flags (void) const;

	/// Set the creation flags.
	void creation_flags (u_long);

	// = <ACE_Process> uses these operations to retrieve option values.

	/// Current working directory.  Returns "" if nothing has been set.
	char *working_directory (void);

	/// Buffer of command-line options.  Returns a pointer to a buffer that
	/// contains the list of command line options.  Prior to a call to
	/// command_line_argv(), this is a single string of space separated
	/// arguments independent of which form of command_line() was used to
	/// create it.  After a call to command_line_argv(), this is a list of
	/// strings each terminated by '\0'.  [Note: spawn() will call
	/// command_line_argv().]  The total length of all these strings is the
	/// same as the single string in the prior case and can be obtained by
	/// providing max_len. @arg max_len, if non-zero, provides a location
	/// into which the total length of the command line buffer is returned.
	char *command_line_buf (int *max_len = 0);

	/**
	* argv-style command-line options.  Parses and modifies the string
	* created from <command_line_>.  All spaces not in quotes ("" or
	* '') are replaced with null (\0) bytes.  An argv array is built
	* and returned with each entry pointing to the start of
	* null-terminated string.  Returns { 0 } if nothing has been set.
	*/
	char * const *command_line_argv (void);

	/**
	* Null-terminated buffer of null terminated strings.  Each string
	* is an environment assignment "VARIABLE=value".  This buffer
	* should end with two null characters.
	*/
	char *env_buf (void);


	/// Allows disabling of handle inheritence, default is TRUE.
	int handle_inheritence (void);
	void handle_inheritence (int);

	/// Cause the specified handle to be passed to a child process
	/// when it runs a new program image.
	/**
	* The specified handle value will be included in the spawned
	* process's command line as @arg +H @arg handle, if a new
	* program is spawned (always on Win32; else if NO_EXEC is not
	* set in creation flags).  The passed handle value will be
	* duplicated if on Win32 less capable than NT.
	* @return 0 if success, -1 if failure.
	*/
	int pass_handle (AX_HANDLE);

	/// Get a copy of the handles the ACE_Process_Options duplicated
	/// for the spawned process.
	/**
	* Any handles created through duplication of those passed into
	* @arg pass_handle are returned in @arg set.
	* @return 0 if there were no handles to return; 1 if there were.
	*/
	//int dup_handles (ACE_Handle_Set &set) const;

	/// Get a copy of the handles passed to the spawned process. This
	/// will be the set of handles previously passed to @arg pass_handle().
	/**
	* Any handles previously passed to @arg pass_handle are returned
	* in @arg set.
	* @return 0 if there were no handles to return; 1 if there were.
	*/
	//int passed_handles (ACE_Handle_Set &set) const;

	/// Set value for avoid_zombies (has no real effect except on *nix).
	void avoid_zombies (int);

	/// Get current value for avoid_zombies.
	int avoid_zombies (void);

#if defined (_WIN32)
	// = Non-portable accessors for when you "just have to use them."

	/// Used for setting and getting.
	STARTUPINFO *startup_info (void);

	/// Get the process_attributes.  Returns NULL if
	/// set_process_attributes has not been set.
	LPSECURITY_ATTRIBUTES get_process_attributes (void) const;

	/// If this is called, a non-null process attributes is sent to
	/// CreateProcess.
	LPSECURITY_ATTRIBUTES set_process_attributes (void);

	/// Get the thread_attributes.  Returns NULL if set_thread_attributes
	/// has not been set.
	LPSECURITY_ATTRIBUTES get_thread_attributes (void) const;

	/// If this is called, a non-null thread attributes is sent to
	/// CreateProcess.
	LPSECURITY_ATTRIBUTES set_thread_attributes (void);

#else /* All things not WIN32 */

	/// argv-style array of environment settings.
	char *const *env_argv (void);

	// = Accessors for the standard handles.
	AX_HANDLE get_stdin (void) const;
	AX_HANDLE get_stdout (void) const;
	AX_HANDLE get_stderr (void) const;

	// = Set/get real & effective user & group id associated with user.
	int setreugid (const char* user);
	void setruid (uid_t id);
	void seteuid (uid_t id);
	void setrgid (uid_t id);
	void setegid (uid_t id);
	uid_t getruid (void) const;
	uid_t geteuid (void) const;
	uid_t getrgid (void) const;
	uid_t getegid (void) const;

	/**
	* Get the inherit_environment flag.
	*/
	int inherit_environment (void) const;

	/**
	* Set the inherit_environment flag.
	*/
	void inherit_environment (int nv);
#endif /* ACE_WIN32 */
protected:

#if !defined (ACE_HAS_WINCE)
	/// Add @a assignment to environment_buf_ and adjust
	/// environment_argv_.  @a len is the strlen of @a assignment.
	int setenv_i (char *assignment, size_t len);

	/// Whether the child process inherits the current process
	/// environment.
	int inherit_environment_;
#endif /* !ACE_HAS_WINCE */

	/// Default 0.
	u_long creation_flags_;

	/// Avoid zombies for spawned processes.
	int avoid_zombies_;

#if defined (_WIN32)
	/// Helper function to grab win32 environment and stick it in
	/// environment_buf_ using this->setenv_i.
	void inherit_environment (void);

	/// Ensures once only call to inherit environment.
	int environment_inherited_;

	STARTUPINFO startup_info_;

	/// Default TRUE.
	BOOL handle_inheritence_;

	/// Pointer to security_buf1_.
	LPSECURITY_ATTRIBUTES process_attributes_;

	/// Pointer to security_buf2_.
	LPSECURITY_ATTRIBUTES thread_attributes_;

	/// Data for process_attributes_.
	SECURITY_ATTRIBUTES security_buf1_;

	/// Data for thread_attributes_.
	SECURITY_ATTRIBUTES security_buf2_;

#else /* !ACE_WIN32 */
	AX_HANDLE stdin_;
	AX_HANDLE stdout_;
	AX_HANDLE stderr_;

	// = Real & effective user & group id's.
	//   These should be set to -1 to leave unchanged (default).
	uid_t ruid_;
	uid_t euid_;
	uid_t rgid_;
	uid_t egid_;
#endif /* ACE_WIN32 */

	/// Is 1 if stdhandles was called.
	int set_handles_called_;

	/// Pointer into environment_buf_.  This should point to the next
	/// free spot.
	size_t environment_buf_index_;

	/// Pointer to environment_argv_.
	int environment_argv_index_;

	/// Pointer to buffer of the environment settings.
	char *environment_buf_;

	/// Size of the environment buffer. Configurable
	size_t environment_buf_len_;

	/// Pointers into environment_buf_.
	char **environment_argv_;

	/// Maximum number of environment variables. Configurable
	int max_environment_args_;

	/// Maximum index of environment_argv_ buffer
	int max_environ_argv_index_;

	/// The current working directory.
	char working_directory_[MAXPATHLEN + 1];

	/// Ensures command_line_argv is only calculated once.
	int command_line_argv_calculated_;

	/// Pointer to buffer of command-line arguments.  E.g., "-f foo -b bar".
	char *command_line_buf_;

	/// Pointer to copy of command-line arguments, which is needed when
	/// converting a command-line string into a command-line argv.
	char *command_line_copy_;

	/// Max length of command_line_buf_
	int command_line_buf_len_;

	/// Argv-style command-line arguments.
	char *command_line_argv_[MAX_COMMAND_LINE_OPTIONS];

	/// Process-group on Unix; unused on Win32.
	pid_t process_group_;

	/// Set of handles that were passed in pass_handle ().
	//ACE_Handle_Set handles_passed_;

	/// Results of duplicating handles passed in pass_handle ().
	//ACE_Handle_Set dup_handles_;

	/// Pathname for the process. Relative path or absolute path or just
	/// the program name.
	char process_name_[MAXPATHLEN + 1];
};

class  AX_Process
{
public:
	//friend class ACE_Process_Manager;

	/// Default construction.  Must use <ACE_Process::spawn> to start.
	AX_Process (void);

	/// Destructor.
	virtual ~AX_Process (void);

	/**
	* Called just before <ACE_OS::fork> in the <spawn>.  If this
	* returns non-zero, the <spawn> is aborted (and returns
	* ACE_INVALID_PID).  The default simply returns zero.
	*/
	virtual int prepare (AX_Process_Options &options);

	/**
	* Launch a new process as described by @a options. On success,
	* returns 1 if the option avoid_zombies is set, else returns the
	* process id of the newly spawned child. Returns -1 on
	* failure. This will be fixed in the future versions of ACE when
	* the process id of the child will be returned regardless of the option.
	*/
	virtual pid_t spawn (AX_Process_Options &options);

	/// Called just after <ACE_OS::fork> in the parent's context, if the
	/// <fork> succeeds.  The default is to do nothing.
	virtual void parent (pid_t child);

	/**
	* Called just after <ACE_OS::fork> in the child's context.  The
	* default does nothing.  This function is *not* called on Win32
	* because the process-creation scheme does not allow it.
	*/
	virtual void child (pid_t parent);

	/// Called by a <Process_Manager> that is removing this Process from
	/// its table of managed Processes.  Default is to do nothing.
	virtual void unmanage (void);

	/**
	* Wait for the process we've created to exit.  If <status> != 0, it
	* points to an integer where the function store the exit status of
	* child process to.  If <wait_options> == <WNOHANG> then return 0
	* and don't block if the child process hasn't exited yet.  A return
	* value of -1 represents the <wait> operation failed, otherwise,
	* the child process id is returned.
	*/
	pid_t wait (int *status = 0,
		int wait_options = 0);

	/**
	* Timed wait for the process we've created to exit.  A return value
	* of -1 indicates that the something failed; 0 indicates that a
	* timeout occurred.  Otherwise, the child's process id is returned.
	* If <status> != 0, it points to an integer where the function
	* stores the child's exit status.
	*
	* @note On UNIX platforms this function uses <ualarm>, i.e., it
	* overwrites any existing alarm.  In addition, it steals all
	* <SIGCHLD>s during the timeout period, which will break another
	* <ACE_Process_Manager> in the same process that's expecting
	* <SIGCHLD> to kick off process reaping.
	*/
	pid_t wait (const timeval &tv,
		int *status = 0);

	/// Send the process a signal.  This is only portable to operating
	/// systems that support signals, such as UNIX/POSIX.
	int kill (int signum = SIGINT);

	/**
	* Terminate the process abruptly using <ACE::terminate_process>.
	* This call doesn't give the process a chance to cleanup, so use it
	* with caution...
	*/
	int terminate (void);

	/// Return the process id of the new child process.
	pid_t getpid (void) const;

	/// Return the handle of the process, if it has one.
	AX_HANDLE gethandle (void) const;

	/// Return 1 if running; 0 otherwise.
	int running (void) const;

	/// Return the Process' exit code.  This method returns the raw
	/// exit status returned from system APIs (such as <wait> or
	/// <waitpid>).  This value is system dependent.
	int exit_code (void) const;

	/// Return the Process' return value.  This method returns the
	/// actual return value that a child process returns or <exit>s.
	int return_value (void) const;

	/// Close all the handles in the set obtained from the
	/// @arg ACE_Process_Options::dup_handles object used to spawn
	/// the process.
	void close_dup_handles (void);

	/// Close all the handles in the set obtained from the
	/// @arg ACE_Process_Options::passed_handles object used to spawn
	/// the process.
	void close_passed_handles (void);

#if defined (_WIN32)
	PROCESS_INFORMATION process_info (void);
#endif /* ACE_WIN32 */

private:

	// Disallow copying and assignment since we don't support this (yet).
	AX_Process (const AX_Process &);
	void operator= (const AX_Process &);

protected:
	/// Set this process' <exit_code_>.  ACE_Process_Manager uses this
	/// method to set the <exit_code_> after successfully waiting for
	/// this process to exit.
	void exit_code (int code);

#if defined (_WIN32)
	PROCESS_INFORMATION process_info_;
#else /* ACE_WIN32 */
	/// Process id of the child.
	pid_t child_id_;
#endif /* ACE_WIN32 */
	int exit_code_;

	///// Set of handles that were passed to the child process.
	//ACE_Handle_Set handles_passed_;
	///// Handle duplicates made for the child process.
	//ACE_Handle_Set dup_handles_;

};
#include "AX_Process_Option.inl"
#endif
