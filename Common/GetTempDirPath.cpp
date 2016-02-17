#include "stdafx.h"
#include "GetTempDirPath.h"


DWORD GetTempDirPath(
	TCHAR** pszTempDirPath,
	DWORD*  pdwTempDirPathSize)
{
	TCHAR  acTempBuf[2];
	TCHAR* szPathBuf;
	DWORD  dwPathBufSize;
	DWORD  dwPathSize;
	DWORD  dwRetValue;

	if (pszTempDirPath == NULL)
		dwRetValue = ERROR_INVALID_PARAMETER;
	else
	{
		dwPathBufSize = (DWORD) (sizeof(acTempBuf) / sizeof(TCHAR));
		dwPathSize = GetTempPath(dwPathBufSize, acTempBuf);

		if (dwPathSize == 0)
			dwRetValue = GetLastError();
		else
		{
			dwPathBufSize = dwPathSize;
			szPathBuf = (TCHAR*) malloc((size_t) (dwPathBufSize * sizeof(TCHAR)));

			if (szPathBuf == NULL)
				dwRetValue = ERROR_OUTOFMEMORY;
			else
			{
				dwPathSize = GetTempPath(dwPathBufSize, szPathBuf);

				if (dwPathSize == 0)
				{
					dwRetValue = GetLastError();
					free((void*) szPathBuf);
				}
				else
				{
					dwRetValue = ERROR_SUCCESS;
					*pszTempDirPath = szPathBuf;

					if (pdwTempDirPathSize != NULL)
						*pdwTempDirPathSize = dwPathSize;
				}
			}
		}
	}

	if (dwRetValue != ERROR_SUCCESS)
	{
		if (pszTempDirPath != NULL)
			*pszTempDirPath = NULL;

		if (pdwTempDirPathSize != NULL)
			*pdwTempDirPathSize = 0;
	}

	return(dwRetValue);
}
