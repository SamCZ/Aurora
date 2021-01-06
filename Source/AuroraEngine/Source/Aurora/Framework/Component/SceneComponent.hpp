#pragma once

#include "ActorComponent.hpp"

namespace Aurora::Framework
{
    class HSceneComponent : public HActorComponent
    {
    private:
        List<HSceneComponent*> m_ChildrenList;
    public:

    };
}