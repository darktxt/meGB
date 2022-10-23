#pragma once
#include"common.h"
#include"instructionset.h"
class CPU {
public:
	CPU(std::shared_ptr<MMU> m_MMU):m_MMU(m_MMU), m_instructionset(m_MMU){}
	bool Initialize();
	bool LoadRom();
	int step();
private:
	std::shared_ptr<MMU> m_MMU;
	InstructionSet m_instructionset;
	ushort m_PC;
};