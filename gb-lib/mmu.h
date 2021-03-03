#pragma once
#include"munit.h"
#include"common.h"
#include"logger.h"

class MMU: public MUnit {
public:
	bool LoadROM();
	byte readByte(ushort address) const;
private:
	byte memory[0xffff + 1];
};