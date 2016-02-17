#pragma once

#include <Windows.h>
#include <tchar.h>


class BinaryFileWriter
{
public:
	BinaryFileWriter(void);
	virtual ~BinaryFileWriter(void);
	int Open(_TCHAR* filePath);
	int Close(void);

	int Write(
		BYTE*  dataBuffer,
		size_t dataByteCount);

	int Flush(void);

private:
	FILE* fileStream;
};
