#include"Emulator.h"

byte* Emulator::GetCurrentFrame()
{	
	memset(m_DisplayPixels, 0x00, ARRAYSIZE(m_DisplayPixels));
	return m_DisplayPixels;
}
