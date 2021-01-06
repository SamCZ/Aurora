#pragma once

#include <Aurora/Core/Vector.hpp>

#include "../Object.hpp"

namespace Aurora::Framework
{
    PREDEFINE_PTR(class, AActor)
    PREDEFINE_PTR(class, FWorld)

    class HActorComponent : public HObject
    {
    private:
        bool m_IsActive;
    public:
        List<String> Tags;

        AActorPtr Owner;
        FWorldPtr World;
    public:
        HActorComponent() : m_IsActive(true), Owner(nullptr), World(nullptr)
        {

        }
        ~HActorComponent() override = default;

        void SetActive(bool newActive)
        {
            m_IsActive = newActive;
        }

        void ToggleActive()
        {
            m_IsActive = !m_IsActive;
        }

        bool IsActive() const
        {
            return m_IsActive;
        }

        virtual void Tick(double delta) {}

        virtual void BeginPlay() {}
        virtual void BeginDestroy() {}

        virtual Matrix4 GetTransformMatrix()
        {
            return glm::identity<Matrix4>();
        }
    };
}