#include "mmu.h"

bool MMU::LoadROM()
{
	return false;
}


bool MMU::WriteByte(const ushort address, const byte val) {
	return false;
}

byte MMU::ReadByte(const ushort address) const {
	return 0x00;
}

ushort MMU::ReadShort(const ushort address) const
{
	ushort val = ReadByte(address + 1);
	val = val << 8;
	val |= ReadByte(address);
	return val;
}

void MMU::RegisterMUnits(RF readFunc, WF writeFunc, std::pair<ushort, ushort> legalAddress)
{
	for (int i = legalAddress.first; i <= legalAddress.second; i++)
		memory[i] = { readFunc,writeFunc };
}


bool MMU::Write(const ushort address, const byte val)
{
	if (address < 0 || address>0xffff) {
		// never happen
		Logger::GetInstance().LogError("MMU: Write illegally! %x", address);
		return false;
	}

	return memory[address].writeFunc(address, val);
}

byte MMU::Read(ushort address) const
{
	if (address < 0 || address>0xffff) {
		// never happen
		Logger::GetInstance().LogError("MMU: Read illegally! %x", address);
		return 0x00;
	}
	return memory[address].readFunc(address);
}
