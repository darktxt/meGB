#include "cpu.h"
#include "mmu.h"

bool CPU::Initialize()
{
	return false;
}

bool CPU::LoadRom()
{
	return false;
}

int CPU::step()
{
	byte opCode = m_MMU->Read(m_PC);
    m_PC++;

    OPF instruction;
    if (opCode == 0xCB)
    {
        opCode = m_MMU->Read(m_PC);
        m_PC++;
        instruction = m_instructionset.m_operationMapCB[opCode];
    }
    else
    {
        instruction = m_instructionset.m_operationMap[opCode];
    }

    int cycles = 4;

    if (instruction != nullptr)
    {
        cycles = instruction(opCode);
    }
    return cycles;
}
