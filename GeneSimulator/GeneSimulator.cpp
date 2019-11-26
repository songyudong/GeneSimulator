// GeneSimulator.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <vector>
#include <random>
#include <tchar.h>
#include <stdio.h>
#include <set>
#include <list>
#include "redisconnect.h"
#include "Base.h"
#include "GeneRedis.h"
#include "GeneFile.h"
#include "GeneMem.h"
#include "GeneFileMap.h"

#pragma warning(disable:4996)
#pragma warning(disable:4018)

using namespace std;
using std::default_random_engine;
const int MAX_BORN_COUNT = 6;
static int BORN_COUNT = MAX_BORN_COUNT;
const int FIRST_ANCESTORS_COUNT = 10000;
const int INIT_N_COUNT = 2;
const int INIT_H_COUNT = FIRST_ANCESTORS_COUNT - INIT_N_COUNT;
const int GENERATION = 1000;
static default_random_engine rand_gen;

static int64_t iid_gen = 0;
static int64_t sample_ancestor = rand_gen()%FIRST_ANCESTORS_COUNT;
static Policy save_policy = Policy::Mem;
static int g_generation = 0;
static FILE* logFile = nullptr;
static vector<int64_t> populatins;



void SaveHuman(Human& h)
{
	if (save_policy == Policy::FileMap)
		SaveHumanToFileMap(h);
	else if (save_policy == Policy::Redis)
		SaveHumanToRedis(h);
	else if (save_policy == Policy::File)
		SaveHumanToFile(h);
	else if (save_policy == Policy::Mem)
		SaveHumanToMem(h);
}

void ReadHuman(int64_t index, Human& h)
{
	if (save_policy == Policy::FileMap)
		ReadHumanFromFileMap(index, h);
	else if (save_policy == Policy::Redis)
		ReadHumanFromRedis(index, h);
	else if (save_policy == Policy::File)
		ReadHumanFromFile(index, h);
	else if (save_policy == Policy::Mem)
		ReadHumanFromMem(index, h);

}





void Born(Human& parent_m, Human& parent_f, Human& child)
{
	child.sex = (Sex)(rand_gen() % 2);
	child.father = parent_m.index;
	child.mother = parent_f.index;
	child.generation = parent_m.generation + 1;
	child.geneRatioFromNeanderthals = (parent_m.geneRatioFromNeanderthals + parent_f.geneRatioFromNeanderthals) / 2;
	if (parent_m.sample_ancestor_flag || parent_f.sample_ancestor_flag)
		child.sample_ancestor_flag = 1;
	SaveHuman(child);
}

void Propagate(vector<int64_t>& parent_m, vector<int64_t>& parent_f, vector<int64_t>& children_m, vector<int64_t>& children_f)
{	
	for (int64_t i = 0; i < (int64_t)parent_f.size(); i++)
	{
		Human female;
		ReadHuman(parent_f[i], female);
		int64_t index = rand_gen() % parent_m.size();
		Human male;
		ReadHuman(parent_m[index], male);
		int count = rand_gen() % BORN_COUNT;
		for (int c = 0; c < count; c++)
		{
			Human child;
			child.index = iid_gen++;
			Born(male, female, child);
			if (child.sex == Sex::Male)
				children_m.push_back(child.index);
			else
				children_f.push_back(child.index);
		}	

	}
}

void QueryAncestors(int64_t humanIndex, list<int64_t>& ancestors);
void QueryAncestors(list<int64_t>& ancestors)
{
	while (true)
	{
		auto itr = ancestors.begin();
		if (*itr < FIRST_ANCESTORS_COUNT)
			return;

		list<int64_t> temp = ancestors;
		ancestors.clear();
		for (auto itr=temp.begin(); itr != temp.end(); ++itr)
		{
			Human h;
			ReadHuman(*itr, h);
			int64_t father_index = h.father;
			int64_t mother_index = h.mother;

			if (father_index >= 0 && mother_index >= 0)
			{
				ancestors.push_back(father_index);
				ancestors.push_back(mother_index);
			}
		}

		ancestors.sort();
		ancestors.unique();
	}
}

void QueryAncestors(int64_t humanIndex, list<int64_t>& ancestors)
{
	Human h;
	ReadHuman(humanIndex, h);
	int64_t father_index = h.father;
	int64_t mother_index = h.mother;
	
	if (father_index >= 0 && mother_index >= 0)
	{
		ancestors.push_back(father_index);
		ancestors.push_back(mother_index);		

		QueryAncestors(ancestors);
	}
}





void Run()
{
	vector<int64_t> ancestors_m;
	vector<int64_t> ancestors_f;
	vector<int64_t> children_m;
	vector<int64_t> children_f;

	vector<int64_t> Hs(INIT_H_COUNT);
	for (int i = 0; i < INIT_H_COUNT; i++)
	{
		Human hom;
		hom.index = iid_gen++;
		hom.sex = (Sex)(rand_gen() % 2);
		hom.generation = 0;
		hom.geneRatioFromNeanderthals = 0;
		if (i == sample_ancestor)
			hom.sample_ancestor_flag = 1;
		SaveHuman(hom);
		if (hom.sex == Sex::Male)
			ancestors_m.push_back(hom.index);
		else
			ancestors_f.push_back(hom.index);
	}	

	for (int i = 0; i < INIT_N_COUNT; i++)
	{
		Human nea;
		nea.index = iid_gen++;
		nea.sex = (Sex)(rand_gen() % 2);
		nea.generation = 0;
		nea.geneRatioFromNeanderthals = 1.0f;
		if (i == sample_ancestor)
			nea.sample_ancestor_flag = 1;
		SaveHuman(nea);
		if (nea.sex == Sex::Male)
			ancestors_m.push_back(nea.index);
		else
			ancestors_f.push_back(nea.index);
	}


	for (int i = 0; i < GENERATION; i++)
	{
		g_generation = i;
		populatins.push_back(ancestors_m.size() + ancestors_f.size());
		if (g_generation > 1)
		{
			if (populatins[i - 1] * 1.0f / populatins[0] < std::powf(1.003074831f, i-1))
				BORN_COUNT = MAX_BORN_COUNT;
			else
				BORN_COUNT = MAX_BORN_COUNT - 1;
		}

		printf("\rgeneration:%d/%d, total human:%lld", i, GENERATION, ancestors_m.size()+ancestors_f.size());
		Propagate(ancestors_m, ancestors_f, children_m, children_f);

		ancestors_m.clear();
		for (int64_t c = 0; c < children_m.size(); c++)
		{
			ancestors_m.emplace_back(children_m[c]);
		}
		children_m.clear();


		ancestors_f.clear();
		for (int64_t c = 0; c < children_f.size(); c++)
		{
			ancestors_f.emplace_back(children_f[c]);
		}
		children_f.clear();
	}

	int64_t count_m = ancestors_m.size();
	int64_t count_f = ancestors_f.size();
	int64_t count_m_n = 0;
	int64_t count_f_n = 0;
	int64_t count_flag_m = 0;
	int64_t count_flag_f = 0;

	printf("\nmale: %lld, female: %lld\n", count_m, count_f);

	logFile = fopen("output.log", "w");

	fprintf(logFile, "First generation total count %d, including N count %d\n", (INIT_H_COUNT+INIT_N_COUNT), INIT_N_COUNT);	
	fprintf(logFile, "After propagate %d generations...\n", GENERATION);
	
	for (int i = 0; i < count_m; i++)
	{
		Human h;
		ReadHuman(ancestors_m[i], h);
		float r = h.geneRatioFromNeanderthals;
		if (r > 0)
		{
			count_m_n++;
		}
		if (h.sample_ancestor_flag)
			count_flag_m++;
		
	}
	fprintf(logFile, "N male total %lld/%lld\n", count_m_n, count_m);	
	for (int i = 0; i < count_f; i++)
	{
		Human h;
		ReadHuman(ancestors_f[i], h);
		float r = h.geneRatioFromNeanderthals;
		if (r > 0)
		{
			count_f_n++;
		}
		if (h.sample_ancestor_flag)
			count_flag_f++;
	}
	int64_t count_flag = count_flag_m + count_flag_f;
	float ratio_flag = count_flag * 100.f / (count_m + count_f);
	fprintf(logFile, "N female total %lld/%lld\n", count_f_n, count_f);
	fprintf(logFile, "sample ancestor %lld has %lld progeny(male %lld, female %lld), ratio:%.2f%%\n", 
		sample_ancestor, count_flag_m + count_flag_f, count_flag_m, count_flag_f, ratio_flag);
	
	int64_t sample_male_index = rand_gen() % ancestors_m.size();
	Human sample_male;
	ReadHuman(ancestors_m[sample_male_index], sample_male);
	printf("look up ancestors for male:%lld\n", ancestors_m[sample_male_index]);
	list<int64_t> ancestors;
	QueryAncestors(sample_male.index, ancestors);	
	int64_t firstAncestorsCount = 0;
	for (auto itr = ancestors.begin(); itr != ancestors.end(); ++itr)
	{
		if (*itr < FIRST_ANCESTORS_COUNT)
			firstAncestorsCount++;		
	}
	fprintf(logFile, "sample last generation male:%lld, N ratio:%f, first gen ancestors count:%lld\n", 
		ancestors_m[sample_male_index], sample_male.geneRatioFromNeanderthals, firstAncestorsCount);

	int64_t sample_female_index = rand_gen() % ancestors_f.size();
	Human sample_female;
	ReadHuman(ancestors_f[sample_female_index], sample_female);
	printf("look up ancestors for female:%lld\n", ancestors_f[sample_female_index]);
	ancestors.clear();
	QueryAncestors(ancestors_f[sample_female_index], ancestors);	
	firstAncestorsCount = 0;
	for (auto itr=ancestors.begin(); itr!=ancestors.end(); ++itr)
	{
		if (*itr < FIRST_ANCESTORS_COUNT)
			firstAncestorsCount++;		
	}
	fprintf(logFile, "sample last generation female:%lld, N ratio:%f, first gen ancestors count:%lld\n", 
		ancestors_f[sample_female_index], sample_female.geneRatioFromNeanderthals, firstAncestorsCount);


	fclose(logFile);
}

int main()
{
	if (save_policy == Policy::FileMap)
	{
		CreateFileMap();		
		Run();
		CloseFileMap();
	}
	else if (save_policy == Policy::Redis)
	{
		RedisInit();		
		Run();
	}	
	else if (save_policy == Policy::File)
	{
		FileInit();		
		Run();
	}
	else if (save_policy == Policy::Mem)
	{
		FileInit();
		MemInit();
		Run();
	}		

	return 0;
}

