#ifndef FILE_ACCESS_ENCRYPTED_H
#define FILE_ACCESS_ENCRYPTED_H

#include "core/os/file_access.h"

class FileAccessEncrypted : public FileAccess {
public:
	enum Mode {
		MODE_READ,
		MODE_WRITE_AES256,
		MODE_MAX
	};

private:
	Mode mode;
	Vector<uint8_t> key;
	bool writing;
	FileAccess *file;
	uint64_t base;
	uint64_t length;
	Vector<uint8_t> data;
	mutable uint64_t pos;
	mutable bool eofed;

public:
	Error open_and_parse(FileAccess *p_base, const Vector<uint8_t> &p_key, Mode p_mode);
	Error open_and_parse_password(FileAccess *p_base, const String &p_key, Mode p_mode);

	virtual Error _open(const String &p_path, int p_mode_flags); ///< open a file
	virtual void close(); ///< close a file
	virtual bool is_open() const; ///< true when file is open

	virtual String get_path() const; /// returns the path for the current open file
	virtual String get_path_absolute() const; /// returns the absolute path for the current open file

	virtual void seek(uint64_t p_position); ///< seek to a given position
	virtual void seek_end(int64_t p_position = 0); ///< seek from the end of file
	virtual uint64_t get_position() const; ///< get position in the file
	virtual uint64_t get_len() const; ///< get size of the file

	virtual bool eof_reached() const; ///< reading passed EOF

	virtual uint8_t get_8() const; ///< get a byte
	virtual uint64_t get_buffer(uint8_t *p_dst, uint64_t p_length) const;

	virtual Error get_error() const; ///< get last error

	virtual void flush();
	virtual void store_8(uint8_t p_dest); ///< store a byte
	virtual void store_buffer(const uint8_t *p_src, uint64_t p_length); ///< store an array of bytes

	virtual bool file_exists(const String &p_name); ///< return true if a file exists

	virtual uint64_t _get_modified_time(const String &p_file);
	virtual uint32_t _get_unix_permissions(const String &p_file);
	virtual Error _set_unix_permissions(const String &p_file, uint32_t p_permissions);

	FileAccessEncrypted();
	~FileAccessEncrypted();
};

#endif // FILE_ACCESS_ENCRYPTED_H
