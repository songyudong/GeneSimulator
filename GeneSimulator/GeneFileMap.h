#pragma once
#include "Base.h"
#include "stdint.h"
#include "windows.h"

void CreateFileMap();
void CloseFileMap();
void SaveHumanToFileMap(Human& h);
void ReadHumanFromFileMap(int64_t index, Human& h);