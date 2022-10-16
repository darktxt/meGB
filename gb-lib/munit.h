#pragma once
#include "common.h"
#include "logger.h"

struct MUnit {
	const std::string name;
	const WF writeFunc;
	const RF readFunc;
	const std::pair<ushort, ushort> legalAddress;
	MUnit(std::string& name, RF readFunc, WF writeFunc, std::pair<ushort,ushort> legalAddress)
		:legalAddress(legalAddress),
		readFunc(readFunc),
		writeFunc(writeFunc),
		name(name)
	{
	}
};