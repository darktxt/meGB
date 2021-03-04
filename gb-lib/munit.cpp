#include "munit.h"

byte MUnit::ReadByte(const ushort address)
{
    for (auto legalAddress : legalAddresses) {
        if (address >= legalAddress[0] && address <= legalAddress[1])
            return readFunc(address);
    }
    Logger::LogError("Read error! Address %x is illegal for %s", address, name.c_str());
    return 0x00;
}

bool MUnit::WriteByte(const ushort address, const byte byte)
{
    for (auto legalAddress : legalAddresses) {
        if (address >= legalAddress[0] && address <= legalAddress[1])
            return writeFunc(address,byte);
    }
    Logger::LogError("Write error! Address %x is illegal for %s", address, name.c_str());
    return false;
}
