#pragma once
#include "common.h"
#include "logger.h"

class MUnit {
public:
	MUnit(std::string name, RF readFunc, WF writeFunc, std::vector<std::array<ushort, 2>> legalAddresses)
		:legalAddresses(legalAddresses),
		readFunc(readFunc),
		writeFunc(writeFunc),
		name(name)
	{
	}


private:
	const std::string name;
	byte ReadByte(const ushort address);
	bool WriteByte(const ushort address, const byte byte);
	const WF writeFunc;
	const RF readFunc;
	const std::vector<std::array<ushort, 2>> legalAddresses;
};