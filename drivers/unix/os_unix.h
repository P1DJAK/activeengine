#ifndef OS_UNIX_H
#define OS_UNIX_H

#ifdef UNIX_ENABLED

#include "core/os/os.h"
#include "drivers/unix/ip_unix.h"

class OS_Unix : public OS {
protected:
	// UNIX only handles the core functions.
	// inheriting platforms under unix (eg. X11) should handle the rest

	virtual void initialize_core();
	virtual int unix_initialize_audio(int p_audio_driver);
	//virtual Error initialize(int p_video_driver,int p_audio_driver);

	virtual void finalize_core();

	String stdin_buf;

public:
	OS_Unix();

	virtual void alert(const String &p_alert, const String &p_title = "ALERT!");
	virtual String get_stdin_string();

	//virtual void set_mouse_show(bool p_show);
	//virtual void set_mouse_grab(bool p_grab);
	//virtual bool is_mouse_grab_enabled() const = 0;
	//virtual void get_mouse_position(int &x, int &y) const;
	//virtual void set_window_title(const String& p_title);

	//virtual void set_video_mode(const VideoMode& p_video_mode);
	//virtual VideoMode get_video_mode() const;
	//virtual void get_fullscreen_mode_list(List<VideoMode> *p_list) const;

	virtual Error open_dynamic_library(const String p_path, void *&p_library_handle, bool p_also_set_library_path = false);
	virtual Error close_dynamic_library(void *p_library_handle);
	virtual Error get_dynamic_library_symbol_handle(void *p_library_handle, const String p_name, void *&p_symbol_handle, bool p_optional = false);

	virtual Error set_cwd(const String &p_cwd);

	virtual String get_name() const;

	virtual Date get_date(bool utc) const;
	virtual Time get_time(bool utc) const;
	virtual TimeZoneInfo get_time_zone_info() const;

	virtual uint64_t get_unix_time() const;
	virtual uint64_t get_system_time_secs() const;
	virtual uint64_t get_system_time_msecs() const;
	virtual double get_subsecond_unix_time() const;

	virtual void delay_usec(uint32_t p_usec) const;
	virtual uint64_t get_ticks_usec() const;

	virtual Error execute(const String &p_path, const List<String> &p_arguments, bool p_blocking = true, ProcessID *r_child_id = nullptr, String *r_pipe = nullptr, int *r_exitcode = nullptr, bool read_stderr = false, Mutex *p_pipe_mutex = nullptr, bool p_open_console = false);
	virtual Error kill(const ProcessID &p_pid);
	virtual int get_process_id() const;
	virtual bool is_process_running(const ProcessID &p_pid) const;

	virtual bool has_environment(const String &p_var) const;
	virtual String get_environment(const String &p_var) const;
	virtual bool set_environment(const String &p_var, const String &p_value) const;
	virtual String get_locale() const;

	virtual void initialize_debugging();

	virtual String get_executable_path() const;
	virtual String get_user_data_dir() const;
};

class UnixTerminalLogger : public StdLogger {
public:
	virtual void log_error(const char *p_function, const char *p_file, int p_line, const char *p_code, const char *p_rationale, ErrorType p_type = ERR_ERROR);
	virtual ~UnixTerminalLogger();
};

#endif

#endif // OS_UNIX_H
