#pragma once


#ifdef PSCSOCKETCONTROL_STATIC_LINK
	#define PSCSOCKETCONTROL_API
#else
	#if defined(PSCSOCKETCONTROL_EXPORTS)
		#if defined(__cplusplus)
			#define PSCSOCKETCONTROL_API extern "C" _declspec(dllexport)
		#else
			#define PSCSOCKETCONTROL_API _declspec(dllexport)
		#endif
	#else
		#if defined(__cplusplus)
			#define PSCSOCKETCONTROL_API extern "C" _declspec(dllimport)
		#else
			#define PSCSOCKETCONTROL_API _declspec(dllimport)
		#endif
	#endif
#endif

bool PSCSocketControl_Initialize(char* szProgramName, char* szProgramVersion);

bool PSCSocketControl_Terminate(void);

bool PSCSocketControl_SetDataLogFilePath(void);