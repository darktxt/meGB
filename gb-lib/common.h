#pragma once
#include <cstring>
#include <vector>
#include <array>
#include <cstdarg>
#include <iostream>
#include <memory>
#include <functional>
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef signed char sbyte;

typedef std::function<bool(const ushort address, const byte byte)> WF;
typedef std::function<byte(const ushort address)> RF;
typedef std::function<ulong(const byte& opCode)> OPF;

#define ARRAYSIZE(a) (sizeof a / sizeof a[0])
#define ISBITSET(val, bit) (((val >> bit) & 0x01) == 0x01)
#define SETBIT(val, bit) (val | (1 << bit))
#define CLEARBIT(val, bit) (val & ~(1 << bit))