#pragma once
#include "Base.h"
#include "stdint.h"

void FileInit();

void SaveHumanToFile(Human& h);

void ReadHumanFromFile(int64_t index, Human& h);

void FileTest();