#include "mmu.h"

bool MMU::LoadROM()
{
	return false;
}

byte MMU::readByte(ushort address) const
{
	if(0<=address&&address<=0xffff)
		return memory[address];
	Logger::LogError("Read illegally! %x", address);
	return 0x00;
}
