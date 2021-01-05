#pragma once

#include <SwapChain.h>
#include <RefCntAutoPtr.hpp>

#include "Window.hpp"

namespace Aurora::App
{
    class FWindowGameContext
    {
    private:
        const FWindowPtr m_Window;
    public:
        explicit FWindowGameContext(FWindowPtr window);
        virtual ~FWindowGameContext() = default;

        virtual void Init() {}
        virtual void Update(double delta, double currentTime) {}
        virtual void Render() {}

    public:
        const FWindowPtr& GetWindow();

        inline RefCntAutoPtr<ISwapChain>& GetSwapChain()
        {
            return GetWindow()->GetSwapChain();
        }
    };
}