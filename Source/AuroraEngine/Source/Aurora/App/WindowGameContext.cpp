#include "WindowGameContext.hpp"

namespace Aurora::App
{
    FWindowGameContext::FWindowGameContext(FWindowPtr window) : m_Window(std::move(window))
    {

    }

    const FWindowPtr& FWindowGameContext::GetWindow()
    {
        return m_Window;
    }
}