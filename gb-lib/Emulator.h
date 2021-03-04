#pragma once
#include"common.h"
#include"cpu.h"
class Emulator {
public:
	byte* GetCurrentFrame();
	Emulator() {
		memset(m_DisplayPixels, 0x00, ARRAYSIZE(m_DisplayPixels));
	}
private:
	std::unique_ptr<CPU> m_CPU;
	byte m_DisplayPixels[160 * 144 * 4];
};