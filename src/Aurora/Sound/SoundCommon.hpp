#pragma once

#ifdef FMOD_SUPPORTED

#include <iostream>
#include <fmod_common.h>
#include <Aurora/Logger/Logger.hpp>

namespace Aurora::Sound
{
    static bool ERRCHECK(const FMOD_RESULT& result)
    {
        if (result != FMOD_OK) {
			AU_LOG_ERROR("FMOD Error! ", FMOD_ErrorString(result))
            return false;
        }

        return true;
    }
}
#endif