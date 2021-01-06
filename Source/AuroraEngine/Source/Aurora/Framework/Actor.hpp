#pragma once

#include "Object.hpp"
#include "Component/ActorComponent.hpp"

namespace Aurora::Framework
{
    class AActor : public HObject
    {
    public:
        AActor();
        ~AActor() override = default;

        virtual void InitializeComponents() {}


    };
}