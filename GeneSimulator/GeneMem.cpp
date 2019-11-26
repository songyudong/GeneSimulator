#include "pch.h"
#include "GeneMem.h"
#include <vector>
#include "GeneFile.h"

using namespace std;
#pragma warning(disable:4996)
#pragma warning(disable:4018)
#define K 1000
#define M K*K
#define B M*K

//const int64_t CAP = 1LL * B;
const int64_t CAP = 100LL * M;
vector<vector<Human>> gMem;
extern int g_generation;
int64_t oldCursorH = 0;

void MemInit()
{
	gMem.resize(5);
	
}

void SaveHumanToMem(Human& h)
{
	int64_t cursorH = h.index / CAP;
	if (cursorH >= gMem.size())
	{
		vector<Human> append;		
		gMem.emplace_back(append);
	}	
	
	gMem[cursorH].push_back(h);
	
	if((cursorH > oldCursorH) && (h.generation-gMem[oldCursorH][CAP-1].generation>1))
	{
		SaveGenerationToFile();
	}
}


void SaveGenerationToFile()
{
	printf("\nbegin to save generation %lld to file\n", oldCursorH);
	for (int64_t i = 0; i < gMem[oldCursorH].size(); ++i)
	{
		SaveHumanToFile(gMem[oldCursorH][i]);
	}
	printf("end save\n");

	gMem[oldCursorH].clear();	
	
	{
		vector<Human> helper;
		gMem[oldCursorH].swap(helper);
	}
	
	
	oldCursorH++;
}

void ReadHumanFromMem(int64_t index, Human& h)
{
	int64_t cursorH = index / CAP;
	int64_t cursorL = index%CAP;
	if (cursorH < oldCursorH)
	{
		ReadHumanFromFile(index, h);
	}
	else
	{
		h = gMem[cursorH][cursorL];
	}
	
}

void MemTest()
{

}
