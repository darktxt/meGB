#include"emulator.h"

byte* Emulator::GetCurrentFrame()
{	
	return m_DisplayPixels;
}

bool Emulator::Initialize()
{
	m_MMU = std::make_shared<MMU>();
	m_CPU = std::make_unique<CPU>(m_MMU);
	m_GPU = std::make_unique<GPU>();
	

	auto mmu_read = std::bind(&MMU::ReadByte, m_MMU.get(), std::placeholders::_1);
	auto mmu_write = std::bind(&MMU::WriteByte, m_MMU.get(), std::placeholders::_1, std::placeholders::_2);
	m_MMU->RegisterMUnits(mmu_read, mmu_write, { 0,0xffff });

	auto gpu_read = std::bind(&GPU::ReadByte, m_GPU.get(), std::placeholders::_1);
	auto gpu_write = std::bind(&GPU::WriteByte, m_GPU.get(), std::placeholders::_1, std::placeholders::_2);
	m_MMU->RegisterMUnits(gpu_read, gpu_write, { 0,0x1111 });
	return false;
}
