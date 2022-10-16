#include <gtest/gtest.h>
#include <cpu.h>
// Demonstrate some basic assertions.
TEST(MMUTest, AccessGPU) {
	// Expect equality.
	CPU m_cpu;
	m_cpu.Initialize();
	EXPECT_EQ(0, m_cpu.m_MMU->ReadByte(0xffff));
	EXPECT_EQ(2, m_cpu.m_MMU->ReadByte(0x1111));
}