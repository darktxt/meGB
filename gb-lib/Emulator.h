#pragma once
#include"common.h"

class Emulator {
public:
	byte* GetCurrentFrame();
private:
	byte m_DisplayPixels[160 * 144 * 4];
};