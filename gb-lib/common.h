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
typedef bool (*WF)(const ushort address, const byte byte);
typedef byte (*RF)(const ushort address);
typedef ulong (*OPF)(const byte& opCode);
#define ARRAYSIZE(a) (sizeof a / sizeof a[0])