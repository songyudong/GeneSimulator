#pragma once
#include "stdint.h"
#include <sstream>
#include <vector>

enum Sex
{
	Male = 0,
	Female = 1
};

enum Policy
{
	FileMap = 0,
	Redis = 1,
	File = 2,
	Mem = 3
};

class Human
{
public:
	int64_t index;
	int64_t father = -1;
	int64_t mother = -1;
	Sex sex;
	int sample_ancestor_flag = 0;
	int generation;
	float geneRatioFromNeanderthals;

public:
	void serialize(std::stringstream& s)
	{
		s << index;
		s << " ";
		s << father;
		s << " ";
		s << mother;
		s << " ";
		s << (int)sex;
		s << " ";
		s << sample_ancestor_flag;
		s << " ";
		s << generation;
		s << " ";
		s << geneRatioFromNeanderthals;
	}

	void deserialize(std::stringstream& s)
	{	
		s >> index;
		s >> father;
		s >> mother;
		int temp = 0;
		s >> temp;
		sex = (Sex)temp;
		s >> sample_ancestor_flag;
		s >> generation;
		s >> geneRatioFromNeanderthals;
	}

	template <typename T>
	void WriteData(const T& t, std::vector<char>& out)
	{
		size_t size = out.size();
		out.resize(size + sizeof(T));
		*(T*)(&out[size]) = t;
	}

	template <typename T>
	void ReadData(const T& t, std::vector<char>& in, size_t& pos)
	{
		char* p = &(*in.begin());
		memcpy((char*)&t, p + pos, sizeof(T));
		pos += sizeof(T);
	}

#define WRITE_MEMBER(m)  WriteData(m, v)
#define READ_MEMBER(m) ReadData(m, v, pos)

	void serialize(std::vector<char>& v)
	{
		WRITE_MEMBER(index);
		WRITE_MEMBER(father);
		WRITE_MEMBER(mother);
		WRITE_MEMBER(sex);
		WRITE_MEMBER(sample_ancestor_flag);
		WRITE_MEMBER(generation);
		WRITE_MEMBER(geneRatioFromNeanderthals);
	}

	void deserialize(std::vector<char>& v)
	{
		size_t pos = 0;
		READ_MEMBER(index);
		READ_MEMBER(father);
		READ_MEMBER(mother);
		READ_MEMBER(sex);
		READ_MEMBER(sample_ancestor_flag);
		READ_MEMBER(generation);
		READ_MEMBER(geneRatioFromNeanderthals);
	}
};