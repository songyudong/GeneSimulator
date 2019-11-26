#pragma once
#include "Base.h"
#include "stdint.h"


void RedisInit();

void SaveHumanToRedis(Human& h);

void ReadHumanFromRedis(int64_t index, Human& h);

void RedisTest();