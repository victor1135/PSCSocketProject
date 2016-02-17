#pragma once


#include <sys/stat.h>
#include <tchar.h>


int GetFileSize(		// Get size of a file in bytes.
						// Returns 0 if the file-size information is obtained.
						// A return value of ENOENT indicates that the filename or path could not be found.
						// A return value of EINVAL indicates an invalid parameter.
	_TCHAR* filePath,	// Pointer to a string containing the path of existing file.
	_off_t* fileSize);	// Pointer to a location to receive the file size value.
