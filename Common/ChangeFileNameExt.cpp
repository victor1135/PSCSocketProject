#include "StdAfx.h"
#include "ChangeFileNameExt.h"


TCHAR* ChangeFileNameExtension(
	TCHAR* szSourcePath,
	TCHAR* szNewExtension)
{
	TCHAR  acDriveLetter[_MAX_DRIVE];
	TCHAR  acDirPath[_MAX_DIR];
	TCHAR  acBaseFileName[_MAX_FNAME];
	size_t uiReturnPathBufSize;
	TCHAR* szReturnPath = NULL;

	do
	{
		if (szSourcePath == NULL)
			break;

		if (_tcslen(szSourcePath) <= 0)
			uiReturnPathBufSize = 1;
		else
		{
			if (_tsplitpath_s(
				szSourcePath,
				acDriveLetter,
				sizeof(acDriveLetter) / sizeof(TCHAR),
				acDirPath,
				sizeof(acDirPath) / sizeof(TCHAR),
				acBaseFileName,
				sizeof(acBaseFileName) / sizeof(TCHAR),
				NULL,
				0) != 0)
				break;

			uiReturnPathBufSize =
				_tcslen(acDriveLetter) +
				_tcslen(acDirPath) +
				_tcslen(acBaseFileName) +
				1;

			if (szNewExtension != NULL)
				uiReturnPathBufSize += (_tcslen(szNewExtension) + 1);
		}

		szReturnPath = (TCHAR*) malloc(uiReturnPathBufSize * sizeof(TCHAR));
		if (szReturnPath == NULL) break;

		if (uiReturnPathBufSize == 1)
		{
			szReturnPath[0] = 0;
			break;
		}

		if (_tmakepath_s(
			szReturnPath,
			uiReturnPathBufSize,
			acDriveLetter,
			acDirPath,
			acBaseFileName,
			szNewExtension) != 0)
		{
			free((void*) szReturnPath);
			szReturnPath = NULL;
		}
	}
	while(false);

	return(szReturnPath);
}
