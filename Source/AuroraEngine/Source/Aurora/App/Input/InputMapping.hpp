#pragma once

#include <cstdint>
#include <utility>

#include <Aurora/Core/String.hpp>
#include "Keys.h"

namespace Aurora::App
{
    struct InputActionKeyMapping
    {
        String ActionName;
        Key KeyType;
        uint8_t bShift : 1;
        uint8_t bCtrl : 1;
        uint8_t bAlt : 1;
        uint8_t bCmd : 1;

        bool operator==(const InputActionKeyMapping& Other) const
        {
            return (ActionName == Other.ActionName
                    && KeyType == Other.KeyType
                    && bShift == Other.bShift
                    && bCtrl == Other.bCtrl
                    && bAlt == Other.bAlt
                    && bCmd == Other.bCmd);
        }

        InputActionKeyMapping(String InActionName = String_None, const Key InKey = Keys::Invalid, const bool bInShift = false, const bool bInCtrl = false, const bool bInAlt = false, const bool bInCmd = false)
                : ActionName(std::move(InActionName))
                , KeyType(InKey)
                , bShift(bInShift)
                , bCtrl(bInCtrl)
                , bAlt(bInAlt)
                , bCmd(bInCmd)
        {
        }
    };

    struct InputAxisKeyMapping
    {
        String AxisName;
        Key KeyType;
        float Scale;

        bool operator==(const InputAxisKeyMapping& Other) const
        {
            return (AxisName == Other.AxisName
                    && KeyType == Other.KeyType
                    && Scale == Other.Scale);
        }

        InputAxisKeyMapping(String InAxisName = String_None, const Key InKey = Keys::Invalid, const float InScale = 1.f)
                : AxisName(std::move(InAxisName))
                , KeyType(InKey)
                , Scale(InScale)
        {
        }
    };
}