#pragma once

#include <Windows.h>
#include <tchar.h>


// ChangeFileNameExtension - 變更路徑字串的副檔名。
// 傳回值：含有修改過路徑資訊的字串。
//         用來存放回傳字串的儲存空間是這個 Function 經由記憶體配置而取得的，
//         當不需再使用時，必須呼叫 "void free(void *memblock);" 來釋放儲存空間。
//         如果 szSourcePath 不為 NULL 而傳回值卻為 NULL，表示記憶體配置失敗。
TCHAR* ChangeFileNameExtension(
	TCHAR* szSourcePath,	// 要修改的路徑資訊。
	TCHAR* szNewExtension);	// 新的副檔名 (可能有前置句號)。指定 NULL 以從 szSourcePath 移除現有副檔名。
