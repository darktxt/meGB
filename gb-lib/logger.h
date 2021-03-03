#pragma once
#include"common.h"
constexpr auto BUFFERSIZE = 1024;
class Logger {
private:
	Logger(){}
public:
    static void Disable();
    static void Enable();
    static void Log(const char* message, ...);
    static void LogError(const char* message, ...);
    static void LogCharacter(char character);
private:
    static bool isEnabled;
    static char buffer[BUFFERSIZE];
};