#pragma once
#include "Base.h"
#include "stdint.h"

void MemInit();

void SaveHumanToMem(Human& h);

void ReadHumanFromMem(int64_t index, Human& h);

void SaveGenerationToFile();

void MemTest();