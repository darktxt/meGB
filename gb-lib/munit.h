#pragma once
#include "common.h"
#include "logger.h"

class MUnit {
public:
	WF writeFunc;
	RF readFunc;
	MUnit(RF readFunc = nullptr, WF writeFunc = nullptr)
		:readFunc(readFunc),
		writeFunc(writeFunc)
	{
	}
};