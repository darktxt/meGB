#include "mmu.h"

bool MMU::LoadROM()
{
	return false;
}

bool MMU::WriteByte(const ushort address, const byte val)
{
	if (address < 0 || address>0xffff) {
		// never happen
		// Logger::LogError("Write illegally! %x", address);
		return false;
	}
	for (auto& munit : munit_list) {
		short start = munit.legalAddress.first;
		short end = munit.legalAddress.second;
		if (address >= start && address <= end) {
			return munit.writeFunc(address,val);
		}
	}
	memory[address] = val;
	return true;
}


ushort MMU::ReadShort(const ushort address) const
{
	ushort val = ReadByte(address + 1);
	val = val << 8;
	val |= ReadByte(address);
	return val;
}

void MMU::RegisterMUnit(std::string name, RF readFunc, WF writeFunc, std::pair<ushort, ushort> legalAddress)
{
	munit_list.push_back({ name,readFunc,writeFunc,legalAddress });
}

byte MMU::ReadByte(ushort address) const
{
	if (address < 0 || address>0xffff) {
		// never happen
		// Logger::LogError("Read illegally! %x", address);
		return 0x00;
	}
	for (auto& munit : munit_list) {
		short start = munit.legalAddress.first;
		short end = munit.legalAddress.second;
		if (address >= start && address <= end) {
			return munit.readFunc(address);
		}
	}

	return memory[address];
}
