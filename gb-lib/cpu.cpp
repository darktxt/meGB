#include "cpu.h"

bool CPU::Initialize()
{
	m_GPU = std::make_unique<GPU>();
	m_MMU = std::make_unique<MMU>();

	m_MMU->RegisterMUnit("gpu0", std::bind(&GPU::ReadByte,m_GPU.get(),std::placeholders::_1), std::bind(&GPU::WriteByte, m_GPU.get(), std::placeholders::_1, std::placeholders::_2), { 0,0x1111 });
	return false;
}

bool CPU::LoadRom()
{
	return false;
}
