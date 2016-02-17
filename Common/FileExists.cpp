#include "StdAfx.h"
#include <sys/stat.h>
#include "FileExists.h"


bool FileExists(_TCHAR* filePath)
{
	struct _stat fileStatusInfo;

	if (filePath != NULL)
	{
		if (_tstat(filePath, &fileStatusInfo) == 0)
		{
			if (fileStatusInfo.st_mode & _S_IFREG)
				return(true);
		}
	}

	return(false);
}
