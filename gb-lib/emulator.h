#pragma once
#include"common.h"
#include"cpu.h"
#include "mmu.h"
#include "gpu.h";

#ifdef _DEBUG
#include<gtest/gtest_prod.h>
#endif

class Emulator {

#ifdef _DEBUG
	FRIEND_TEST(MMUTest, AccessGPU);
#endif

public:
	byte* GetCurrentFrame();
	Emulator() {
		memset(m_DisplayPixels, 0x00, ARRAYSIZE(m_DisplayPixels));
	}
	bool Initialize();
private:
	std::unique_ptr<CPU> m_CPU;
	std::shared_ptr<MMU> m_MMU;
	std::unique_ptr<GPU> m_GPU;
	byte m_DisplayPixels[160 * 144 * 4];
};