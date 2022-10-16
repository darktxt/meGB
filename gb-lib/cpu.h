#pragma once
#include"common.h"
#include "mmu.h"
#include "gpu.h";

class CPU {
public:
	bool Initialize();
	bool LoadRom();
private:
	std::unique_ptr<MMU> m_MMU;
	std::unique_ptr<GPU> m_GPU;
};