#pragma once
#include <cstring>
#include <vector>
#include <array>
#include <cstdarg>
#include <iostream>
#include <memory>
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef signed char sbyte;

typedef bool (*WF)(const ushort address, const byte byte);
typedef byte (*RF)(const ushort address);
typedef ulong (*OPF)(const byte& opCode);

#define ARRAYSIZE(a) (sizeof a / sizeof a[0])
#define ISBITSET(val, bit) (((val >> bit) & 0x01) == 0x01)
#define SETBIT(val, bit) (val | (1 << bit))
#define CLEARBIT(val, bit) (val & ~(1 << bit))