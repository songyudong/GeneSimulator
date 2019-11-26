#include "pch.h"
#include "GeneFile.h"

using namespace std;
#pragma warning(disable:4996)
#pragma warning(disable:4018)

static FILE* gGeneFile = nullptr;

void FileInit()
{
	gGeneFile = fopen("gene.data", "wb+");
	
}

void SaveHumanToFile(Human& h)
{
	vector<char> save;
	h.serialize(save);
	fseek(gGeneFile, 0, SEEK_END);
	fwrite(&*save.begin(), save.size(), 1, gGeneFile);
}

void ReadHumanFromFile(int64_t index, Human& h)
{
	vector<char> read;
	read.resize(sizeof(Human));
	fseek(gGeneFile, index * sizeof(Human) , SEEK_SET);
	fread(&*read.begin(), sizeof(Human), 1, gGeneFile);
	h.deserialize(read);
}

void FileTest()
{
	Human test;
	Human check;
	test.index = 0;
	test.father = 1;
	test.mother = 2;
	test.generation = 10;
	test.geneRatioFromNeanderthals = 0.25f;
	test.sex = Sex::Male;
	test.sample_ancestor_flag = 1;

	SaveHumanToFile(test);	
	ReadHumanFromFile(test.index, check);
}