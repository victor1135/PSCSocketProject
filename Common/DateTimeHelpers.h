#pragma once


#include <time.h>


#define CURRENT_LOCAL_DATE_TIME_CHAR_STRING_FORMAT "YYYY-MM-DD hh:mm:ss"


void GetCurrentLocalTime(struct tm* currentLocalTime);

size_t GetCurrentLocalDateTimeCharString(
	char*  dateTimeBuffer,
	size_t dateTimeBufferLen);
