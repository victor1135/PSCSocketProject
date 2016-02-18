#include "stdafx.h"

#include "PSCSocketControl.h"



bool PSCSocketControl_Initialize(char* szProgramName, char* szProgramVersion)
{
	PSCSocketControl_SetDataLogFilePath();
	
	return(true);
}