#pragma once
#include"common.h"
constexpr int BUFFERSIZE = 1024;
class Logger {
public:
    static Logger& GetInstance()
    {
        static Logger instance;
        return instance;
    }
    void Disable();
    void Enable();
    void Log(const char* message, ...);
    void LogError(const char* message, ...);
    void LogCharacter(char character);
private:
    bool isEnabled = true;
    char buffer[BUFFERSIZE] = {};

    Logger() { }
    ~Logger() { }
    Logger(const Logger&);
    Logger& operator = (const Logger&);
};