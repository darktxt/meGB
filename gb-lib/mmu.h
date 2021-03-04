#pragma once
#include"munit.h"
#include"common.h"
#include"logger.h"

class MMU: public MUnit {
public:
	bool LoadROM();
	byte ReadByte(const ushort address) const;
	bool WriteByte(const ushort address, const byte val);
	byte Read(const ushort address) const;
	bool Write(const ushort address, const byte val);
	ushort ReadUShort(const ushort address) const;

private:
	byte memory[0xffff + 1];
};