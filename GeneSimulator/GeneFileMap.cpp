#include "pch.h"
#include "GeneFileMap.h"

static HANDLE hShareMem = INVALID_HANDLE_VALUE;
static void * pRawShareAddr = NULL;
static void * pCurAddr = nullptr;


void SaveHumanToFileMap(Human& h)
{
	int64_t* ptr64 = (int64_t*)pCurAddr;
	*ptr64 = h.index;
	ptr64++;
	*ptr64 = h.father;
	ptr64++;
	*ptr64 = h.mother;
	ptr64++;
	*ptr64 = h.sex;
	ptr64++;
	*ptr64 = h.sample_ancestor_flag;
	ptr64++;
	*ptr64 = h.generation;
	ptr64++;
	float* ptrFloat = (float*)ptr64;
	*ptrFloat = h.geneRatioFromNeanderthals;
	ptr64++;
	pCurAddr = ptr64;
}

void ReadHumanFromFileMap(int64_t index, Human& h)
{

	int64_t* ptr64 = (int64_t*)pRawShareAddr;
	ptr64 += index * 7;
	h.index = *ptr64;
	ptr64++;
	h.father = *ptr64;
	ptr64++;
	h.mother = *ptr64;
	ptr64++;
	h.sex = (Sex)*ptr64;
	ptr64++;
	h.sample_ancestor_flag = (int)*ptr64;
	ptr64++;
	h.generation = (int)*ptr64;
	ptr64++;
	float* ptrFloat = (float*)ptr64;
	h.geneRatioFromNeanderthals = *ptrFloat;
	ptr64++;
}

typedef int(*GETLARGEPAGEMINIMUM)(void);

void DisplayError(DWORD dwError)
{
	LPVOID lpvMessageBuffer;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpvMessageBuffer, 0, NULL);

	// Free the buffer allocated by the system
	LocalFree(lpvMessageBuffer);

	ExitProcess(GetLastError());
}
void CreateFileMap()
{
	HINSTANCE  hDll;
	DWORD size;
	GETLARGEPAGEMINIMUM pGetLargePageMinimum;
	// call succeeds only on Windows Server 2003 SP1 or later
	hDll = LoadLibrary(TEXT("kernel32.dll"));


	pGetLargePageMinimum = (GETLARGEPAGEMINIMUM)GetProcAddress(hDll,
		"GetLargePageMinimum");


	size = (*pGetLargePageMinimum)();

	FreeLibrary(hDll);

	int nMemSizeH = 8;
	hShareMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE/* | SEC_COMMIT | SEC_LARGE_PAGES*/, nMemSizeH, 0, NULL);
	if ((hShareMem != INVALID_HANDLE_VALUE) && (hShareMem != NULL))
	{
		pRawShareAddr = (void *)MapViewOfFile(hShareMem, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		pCurAddr = pRawShareAddr;
	}
	else
	{
		DisplayError(GetLastError());
	}
}

void CloseFileMap()
{
	UnmapViewOfFile(pRawShareAddr);
	pRawShareAddr = NULL;

	CloseHandle(hShareMem);
	hShareMem = INVALID_HANDLE_VALUE;
}

void TestFileMap()
{
	Human test;
	Human check;
	test.index = 23;
	test.father = 1;
	test.mother = 2;
	test.generation = 10;
	test.geneRatioFromNeanderthals = 0.25f;
	test.sex = Sex::Male;
	test.sample_ancestor_flag = 1;
	SaveHumanToFileMap(test);
	SaveHumanToFileMap(test);
	pCurAddr = pRawShareAddr;
	ReadHumanFromFileMap(1, check);

	if (check.index != test.index || check.father != test.father || check.mother != test.mother
		|| check.generation != test.generation || check.sex != test.sex || check.sample_ancestor_flag != test.sample_ancestor_flag)

		printf("error check");
}