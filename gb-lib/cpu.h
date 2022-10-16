#pragma once
#include"common.h"
#include "mmu.h"
#include "gpu.h";

#ifdef _DEBUG
#include<gtest/gtest_prod.h>
#endif

class CPU {
#ifdef _DEBUG
	FRIEND_TEST(MMUTest, AccessGPU);
#endif
public:
	bool Initialize();
	bool LoadRom();
private:
	std::unique_ptr<MMU> m_MMU;
	std::unique_ptr<GPU> m_GPU;
};