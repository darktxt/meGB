#pragma once
#include"common.h"

class Emulator {
public:
	byte* GetCurrentFrame();
	Emulator() {
		memset(m_DisplayPixels, 0x00, ARRAYSIZE(m_DisplayPixels));
	}
private:
	byte m_DisplayPixels[160 * 144 * 4];
};