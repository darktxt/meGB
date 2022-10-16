#pragma once
#include"common.h"
#include "mmu.h"
#include "gpu.h";

#include<gtest/gtest_prod.h>

class CPU {
	FRIEND_TEST(MMUTest, AccessGPU);
public:
	bool Initialize();
	bool LoadRom();
private:
	std::unique_ptr<MMU> m_MMU;
	std::unique_ptr<GPU> m_GPU;
};