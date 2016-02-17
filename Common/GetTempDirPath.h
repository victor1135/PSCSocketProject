#pragma once

#include <stdlib.h>
#include <malloc.h>
#include <Windows.h>


// GetTempDirPath - Retrieves the path of the directory designated for temporary files.
// If the function succeeds, the return value is zero (ERROR_SUCCESS).
// If the function fails, the return value is a nonzero system error code.
DWORD GetTempDirPath(
	TCHAR** pszTempDirPath,			// [out] A pointer to the pointer of the string buffer that is allocated by this
									//       function and receives the null-terminated string specifying the temporary
									//       file path.
									//       The returned string ends with a backslash, for example, C:\TEMP\.
									//       The allocated buffer should be freed by calling the (void free(void *memblock);)
									//       function when it is not needed any more.
	DWORD*  pdwTempDirPathSize);	// [out] A pointer to the length of the Temp directory path string, in TCHARs,
									//       not including the terminating null character.
