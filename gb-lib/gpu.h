#pragma once
#include "common.h"
#include "munit.h"
class GPU  {
public:
	byte ReadByte(const ushort address);
	bool WriteByte(const ushort address, const byte byte);
};