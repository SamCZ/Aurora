#pragma once

#ifdef FMOD_SUPPORTED

#include <iostream>
#include <fmod_common.h>

namespace Aurora::Sound
{
    static bool ERRCHECK(const FMOD_RESULT& result)
    {
        if (result != FMOD_OK) {
            std::cout << "FMOD Error! " << FMOD_ErrorString(result) << std::endl;
            return false;
        }

        return true;
    }
}
#endif