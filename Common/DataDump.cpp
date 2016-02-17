#include "StdAfx.h"
#include <strsafe.h>
#include "DataDump.h"


#define DATA_DESCRIPTION_LINE_FORMAT_STRING "%s (Length = %u / 0x%X)\r\n"
#define NUMBER_OF_DATA_BYTES_PER_DUMP_LINE  16
#define DATA_BYTE_OFFSET_FORMAT_STRING      "%5u (%04X): "
#define DATA_BYTE_OFFSET_LAYOUT             "DDDDD (XXXX): "
#define HEX_DATA_DUMP_LAYOUT                "XX XX XX XX XX XX XX XX - XX XX XX XX XX XX XX XX "
//                                          "|................|"
#define CHAR_DATA_DUMP_SIDELINE             '|'

typedef struct
{
	char dataByteOffset[sizeof(DATA_BYTE_OFFSET_LAYOUT) - 1];
	char hexDataDump[sizeof(HEX_DATA_DUMP_LAYOUT) - 1];
	char charLeftSideline;
	char charDataDump[NUMBER_OF_DATA_BYTES_PER_DUMP_LINE];
	char charRightSideline;
	char charCR;			// Carriage Return, 0x0D, '\r'
	char charLF;			// Line Feed, 0x0A, '\n'
}   DATA_DUMP_LINE;


size_t DumpData(				// Returns number of characters (bytes) written into the returned buffer (outputData),
								// excluding the terminating null character.
	char*  dataDescription,		// [in]  Pointer to a text string describing the property of the data bytes to be dumped.
	char*  dataBytes,			// [in]  Pointer to the data bytes to be dumped.
	size_t dataByteCount,		// [in]  Number of bytes in the given data buffer to be dumped.
	char** outputData)			// [out] A pointer to the pointer of the buffer that is allocated by this function
								//       and receives the output data.
								//       The allocated buffer should be freed by calling the (void free(void *memblock);)
								//       function when it is not needed any more.
{
	size_t          dataDescStringLen;
	size_t          dataDescLineLen;
	size_t          numberOfDumpLines;
	size_t          numberOfDumpChars;
	char*           outputBuffer;
	size_t          outputDataLen;
	HRESULT         resultValue;
	DATA_DUMP_LINE* dataDumpLine;
	size_t          dataByteOffset;
	char*           hexDataDump;
	char*           charDataDump;
	char            currentDataByte;
	char            currentHexValue;
	size_t          i;

	dataDescStringLen = strlen(dataDescription);
	dataDescLineLen = sizeof(DATA_DESCRIPTION_LINE_FORMAT_STRING) + dataDescStringLen + 16;
	numberOfDumpLines = dataByteCount / NUMBER_OF_DATA_BYTES_PER_DUMP_LINE;

	if ((dataByteCount % NUMBER_OF_DATA_BYTES_PER_DUMP_LINE) > 0)
		numberOfDumpLines += 1;

	numberOfDumpChars = sizeof(DATA_DUMP_LINE) * numberOfDumpLines;
	outputBuffer = (char*) malloc(dataDescLineLen + numberOfDumpChars + sizeof('\r') + sizeof('\n') + sizeof('\0'));

	if (outputBuffer == NULL)
		outputDataLen = 0;
	else
	{
		resultValue = StringCchPrintfA(
			(STRSAFE_LPSTR) outputBuffer,
			dataDescLineLen,
			DATA_DESCRIPTION_LINE_FORMAT_STRING,
			dataDescription,
			(int) dataByteCount,
			(int) dataByteCount);

		if (FAILED(resultValue))
			outputDataLen = 0;
		else
		{
			dataDescLineLen = strlen(outputBuffer);

			if (numberOfDumpLines <= 0)
				outputDataLen = dataDescLineLen;
			else
			{
				dataDumpLine = (DATA_DUMP_LINE*) (outputBuffer + dataDescLineLen);
				memset((void*) dataDumpLine, (int) 0x20, numberOfDumpChars);
				dataByteOffset = 0;

				do
				{
					resultValue = StringCchPrintfA(
						(STRSAFE_LPSTR) &dataDumpLine->dataByteOffset[0],
						sizeof(dataDumpLine->dataByteOffset) + 1,
						DATA_BYTE_OFFSET_FORMAT_STRING,
						(int) dataByteOffset,
						(int) dataByteOffset);

					if (FAILED(resultValue))
					{
						outputDataLen = 0;
						break;
					}

					hexDataDump = &dataDumpLine->hexDataDump[0];
					charDataDump = &dataDumpLine->charDataDump[0];

					for (i = 0; i < NUMBER_OF_DATA_BYTES_PER_DUMP_LINE; i++)
					{
						if (i == (NUMBER_OF_DATA_BYTES_PER_DUMP_LINE / 2))
						{
							*hexDataDump = '-';
							hexDataDump += 2;
						}

						if (dataByteOffset >= dataByteCount)
						{
							*hexDataDump++ = '*';
							*hexDataDump = '*';
						}
						else
						{
							currentDataByte = *(dataBytes + dataByteOffset);
							currentHexValue = (currentDataByte >> 4) & 0x0F;

							if (currentHexValue < 10)
								*hexDataDump = currentHexValue + '0';
							else
								*hexDataDump = currentHexValue - 10 + 'A';

							hexDataDump += 1;
							currentHexValue = currentDataByte & 0x0F;

							if (currentHexValue < 10)
								*hexDataDump = currentHexValue + '0';
							else
								*hexDataDump = currentHexValue - 10 + 'A';

							if (((unsigned char) currentDataByte) < 0x20)
								*charDataDump = '.';
							else
								*charDataDump = currentDataByte;

							charDataDump += 1;
							dataByteOffset += 1;
						}

						hexDataDump += 2;
					}

					dataDumpLine->charLeftSideline = CHAR_DATA_DUMP_SIDELINE;
					dataDumpLine->charRightSideline = CHAR_DATA_DUMP_SIDELINE;
					dataDumpLine->charCR = '\r';
					dataDumpLine->charLF = '\n';
					dataDumpLine += 1;
					numberOfDumpLines--;

					if (numberOfDumpLines <= 0)
					{
						outputDataLen = (size_t) (((char*) dataDumpLine) - outputBuffer);
						break;
					}
				}
				while(true);
			}
		}
	}

	if (outputDataLen > 0)
	{
		outputBuffer[outputDataLen] = '\0';
		*outputData = outputBuffer;
	}
	else
	{
		if (outputBuffer != NULL)
			free((void*) outputBuffer);

		*outputData = NULL;
	}

	return(outputDataLen);
}
