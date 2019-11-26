#include "pch.h"
#include "GeneRedis.h"
#include "redisconnect.h"

static shared_ptr<RedisConnect> gRedis;
using namespace std;
const string KEY_NAME = "gene";

void RedisInit()
{
	RedisConnect::Setup("127.0.0.1", 6379, "gene");
	gRedis = RedisConnect::Instance();
	gRedis->del(KEY_NAME);
}

void RedisTest()
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

	SaveHumanToRedis(test);
	ReadHumanFromRedis(test.index, check);
}

void SaveHumanToRedis(Human& h)
{
	stringstream ss;
	h.serialize(ss);
	
	gRedis->set(to_string(h.index), ss.str());
}

void ReadHumanFromRedis(int64_t index, Human& h)
{	
	string s;
	gRedis->get(to_string(index), s);	
	stringstream ss(s);

	h.deserialize(ss);
}