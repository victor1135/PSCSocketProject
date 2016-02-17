#include "StdAfx.h"
#include <stdio.h>
#include <errno.h>
#include "BinaryFileWriter.h"
#include <Windows.h>

BinaryFileWriter::BinaryFileWriter(void)
{
	fileStream = NULL;
}


BinaryFileWriter::~BinaryFileWriter(void)
{
	Close();
}


int BinaryFileWriter::Open(_TCHAR* filePath)    // Returns 0 if successful; an error code on failure.
{
	_TCHAR* openMode = _T("abS");
	int     retCode;

	Close();
	retCode = (int) _tfopen_s(&fileStream, filePath, openMode);

	if (retCode != 0)
		fileStream = NULL;

	return(retCode);
}


int BinaryFileWriter::Close(void)    // Returns 0 if the file is successfully closed.
									 // Returns EOF to indicate an error.
{
	int retCode;

	if (fileStream == NULL)
		retCode = 0;
	else
	{
		retCode = fclose(fileStream);
		fileStream = NULL;
	}

	return(retCode);
}


int BinaryFileWriter::Write(    // Writes data to the opened binary file.
								// Returns 0 if successful; an error code on failure.
	BYTE*  dataBuffer,          // dataBuffer: Pointer to data to be written.
	size_t dataByteCount)       // dataByteCount: Number of data bytes to be written.
{
	size_t writtenByteCount;
	int    retCode;

	if ((fileStream == NULL) || (dataByteCount == 0))
		retCode = ENOTTY;
	else
	{
		writtenByteCount = fwrite((void*) dataBuffer, sizeof(BYTE), dataByteCount, fileStream);

		if (writtenByteCount == dataByteCount)
			retCode = 0;
		else
			retCode = EIO;
	}

	return(retCode);
}


int BinaryFileWriter::Flush(void)	// Flushes data to the opened binary file.
									// Returns 0 if the buffer was successfully flushed.
									// The value 0 is also returned in case in which the specified stream has no buffer.
{
	int retCode;

	if (fileStream == NULL)
		retCode = ENOTTY;
	else
		retCode = fflush(fileStream);

	return(retCode);
}
