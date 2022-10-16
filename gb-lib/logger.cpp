#include "logger.h"
bool Logger::isEnabled = true;
char Logger::buffer[BUFFERSIZE] = {};
void Logger::Disable()
{
    isEnabled = false;
}

void Logger::Enable()
{
    isEnabled = true;
}

void Logger::Log(const char* message, ...)
{
    if (!isEnabled) return;
    va_list argPointer;
    va_start(argPointer, message);
    std::vsnprintf(buffer, BUFFERSIZE, message, argPointer);
    va_end(argPointer);
    std::cout << buffer << std::endl;
}

void Logger::LogError(const char* message, ...)
{
    if (!isEnabled) return;
    va_list argPointer;
    va_start(argPointer, message);
    std::vsnprintf(buffer, BUFFERSIZE, message, argPointer);
    va_end(argPointer);
    std::cerr << buffer << std::endl;
}

void Logger::LogCharacter(char character)
{
    if (!isEnabled) return;
    std::cout << character << std::flush;
}
