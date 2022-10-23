#pragma once
#include"munit.h"
#include"common.h"
#include"logger.h"

class MMU {
public:
	bool LoadROM();
	bool Write(const ushort address, const byte val);
	byte Read(const ushort address) const;
	void RegisterMUnits(RF readFunc, WF writeFunc, std::pair<ushort, ushort> legalAddress);
	byte ReadByte(const ushort address) const;
	bool WriteByte(const ushort address, const byte val);
	ushort ReadShort(const ushort address) const;

private:
	byte mmu_memory[0xffff + 1] = {};// memory except gpu
	MUnit memory[0xffff + 1];

};