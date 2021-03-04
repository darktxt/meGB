#include "mmu.h"

bool MMU::LoadROM()
{
	return false;
}

bool MMU::WriteByte(const ushort address, const byte val)
{
	return byte();
}

byte MMU::Read(const ushort address) const
{
	return ReadByte(address);
}

bool MMU::Write(const ushort address, const byte val)
{
	return byte();
}

ushort MMU::ReadUShort(const ushort address) const
{
	ushort val = Read(address + 1);
	val = val << 8;
	val |= Read(address);
	return val;
}

byte MMU::ReadByte(ushort address) const
{
	if(0<=address&&address<=0xffff)
		return memory[address];
	Logger::LogError("Read illegally! %x", address);
	return 0x00;
}
