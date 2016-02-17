#include "StdAfx.h"
#include <strsafe.h>
#include "DateTimeHelpers.h"


void GetCurrentLocalTime(struct tm* currentLocalTime)
{
	if (currentLocalTime != NULL)
	{
		time_t systemTime;

		_tzset();
		time(&systemTime);
		localtime_s(currentLocalTime, (const time_t*) &systemTime);
	}
}


// Returns number of characters (bytes) written into the given buffer (dateTimeBuffer),
// excluding the terminating null character.
size_t GetCurrentLocalDateTimeCharString(
	char*  dateTimeBuffer,
	size_t dateTimeBufferLen)
{
	struct tm currentLocalTime;
	HRESULT   resultValue;
	size_t    dateTimeStringLen;

	GetCurrentLocalTime(&currentLocalTime);

	resultValue = StringCchPrintfA(
			(STRSAFE_LPSTR) dateTimeBuffer,
			dateTimeBufferLen,
			"%04u-%02u-%02u %02u:%02u:%02u",
			currentLocalTime.tm_year + 1900,
			currentLocalTime.tm_mon + 1,
			currentLocalTime.tm_mday,
			currentLocalTime.tm_hour,
			currentLocalTime.tm_min,
			currentLocalTime.tm_sec);

	if (FAILED(resultValue))
		dateTimeStringLen = 0;
	else
		dateTimeStringLen = strlen(dateTimeBuffer);

	return(dateTimeStringLen);
}
