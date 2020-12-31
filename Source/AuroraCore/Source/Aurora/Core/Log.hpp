#pragma once

#include <iostream>
#include <assert.h>

#include "String.hpp"

namespace Aurora
{
    inline static void Log(const String& message)
    {
        std::cout << message << std::endl;
    }

    inline static void Log(const String& funcName, const String& message)
    {
        std::cout << funcName << ": " << message << std::endl;
    }

    inline static void Log(const String& funcName, const String& argument, const String& message)
    {
        std::cout << funcName << "(" << argument << "): " << message << std::endl;
    }

    inline static void LogError(const String& funcName)
    {
#ifdef DEBUG
        std::cerr << funcName << std::endl;
        throw;
#else
        std::cout << funcName << std::endl;
#endif
    }

    inline static void LogError(const String& funcName, const String& message)
    {
#ifdef DEBUG
        std::cerr << funcName << ": " << message << std::endl;
        throw;
#else
        std::cout << funcName << ": " << message << std::endl;
#endif

    }

    inline static void LogError(const String& funcName, const String& argument, const String& message)
    {
#ifdef DEBUG
        std::cerr << funcName << "(" << argument << "): " << message << std::endl;
        throw;
#else
        std::cout << funcName << "(" << argument << "): " << message << std::endl;
#endif

    }
}