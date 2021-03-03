#pragma once
#include"common.h"
#include "mmu.h"

class CPU {
public:
	bool Initialize();
	bool LoadRom();
private:
	std::unique_ptr<MMU> m_MMU;
};