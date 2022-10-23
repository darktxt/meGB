#ifdef _DEBUG
#include <gtest/gtest.h>
#include <emulator.h>
// Demonstrate some basic assertions.
TEST(MMUTest, AccessGPU) {

	Emulator m_emulator;
	m_emulator.Initialize();
	EXPECT_EQ(0, m_emulator.m_MMU->Read(0xffff));
	EXPECT_EQ(2, m_emulator.m_MMU->Read(0x1111));
}

#endif 