#pragma once

#include <Windows.h>
#include <tchar.h>


// ChangeFileNameExtension - �ܧ���|�r�ꪺ���ɦW�C
// �Ǧ^�ȡG�t���ק�L���|��T���r��C
//         �ΨӦs��^�Ǧr�ꪺ�x�s�Ŷ��O�o�� Function �g�ѰO����t�m�Ө��o���A
//         ���ݦA�ϥήɡA�����I�s "void free(void *memblock);" �������x�s�Ŷ��C
//         �p�G szSourcePath ���� NULL �ӶǦ^�ȫo�� NULL�A��ܰO����t�m���ѡC
TCHAR* ChangeFileNameExtension(
	TCHAR* szSourcePath,	// �n�ק諸���|��T�C
	TCHAR* szNewExtension);	// �s�����ɦW (�i�঳�e�m�y��)�C���w NULL �H�q szSourcePath �����{�����ɦW�C
