#pragma once

#include <Aurora/Core/String.hpp>
#include <Aurora/Core/SmartPointer.hpp>

namespace Aurora::Framework
{
    class HObject : public SharedFromThis<HObject>
    {
    public:
        String Name;
    public:
        ~HObject() override = default;

        template<class T>
        inline bool IsA() const
        {
            return dynamic_cast<const T*>(this) != nullptr;
        }

        template<typename T>
        inline T* SafeCast()
        {
            return dynamic_cast<T*>(this);
        }

        template<typename T>
        inline const T* SafeConstCast() const
        {
            return dynamic_cast<const T*>(this);
        }
    };
}