#include "StdAfx.h"
#include <errno.h>
#include "GetFileSize.h"


int GetFileSize(
	_TCHAR* filePath,
	_off_t* fileSize)
{
	struct _stat statBuffer;
	int          returnValue;

	if ((returnValue = _tstat(filePath, &statBuffer)) == 0)
		*fileSize = statBuffer.st_size;
	else
	{
		*fileSize = 0;
		returnValue = errno;
	}

	return(returnValue);
}
