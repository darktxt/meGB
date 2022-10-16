#pragma once
#include"munit.h"
#include"common.h"
#include"logger.h"

class MMU {
public:
	bool LoadROM();
	byte ReadByte(const ushort address) const;
	bool WriteByte(const ushort address, const byte val);
	ushort ReadShort(const ushort address) const;
	void RegisterMUnit(std::string name, RF readFunc, WF writeFunc, std::pair<ushort, ushort> legalAddress);

private:
	byte memory[0xffff + 1];
	std::vector<MUnit> munit_list;
};