#pragma once

#include <GLFW/glfw3.h>

#include <Aurora/Core/String.hpp>
#include <Aurora/Core/SmartPointer.hpp>
#include <Aurora/Core/Sizeable.hpp>
#include "CursorMode.hpp"

#include <SwapChain.h>
#include <RefCntAutoPtr.hpp>
#include <utility>

using namespace Diligent;

namespace Aurora::App
{
    class FInputManager;
    PREDEFINE_PTR(class, FInputManager)

    struct FWindowDefinition
    {
        int Width;
        int Height;

        bool HasOSWindowBorder;
        bool Maximized;

        String Title;
    };

    class FWindow : public FSizeable
    {
    private:
        static bool IS_GLFW_CONTEXT_INITIALIZED;
    private:
        String m_Title;
        GLFWwindow* m_WindowHandle;
        bool m_Focused;
        ECursorMode m_CursorMode;
        bool m_Vsync;
    private:
        FInputManagerPtr m_InputManager;
        RefCntAutoPtr<ISwapChain> m_SwapChain;
    public:
        FWindow();
        ~FWindow() override;

        void Initialize(const FWindowDefinition& windowDefinition, const SharedPtr<FWindow>& parentWindow);

        void Show();
        void Hide();

        void Destroy();

        void Minimize();
        void Maximize();
        void Restore();

        void Focus();

        void SetTitle(const String& title);

        [[nodiscard]] inline String GetOriginalTitle() const
        {
            return m_Title;
        }

        inline void SetVsync(bool enabled)
        {
            m_Vsync = enabled;
        }

        [[nodiscard]] inline bool IsVsyncEnabled() const
        {
            return m_Vsync;
        }

        [[nodiscard]] bool IsFocused() const;

        [[nodiscard]] bool IsShouldClose() const;

        virtual GLFWwindow* GetWindowHandle();

        void SetCursorMode(const ECursorMode& mode);
        [[nodiscard]] const ECursorMode& GetCursorMode() const;

        bool IsIconified();

        FInputManagerPtr GetInputManager();
    public:
        inline void SetSwapChain(RefCntAutoPtr<ISwapChain> swapChain)
        {
            if(m_SwapChain != nullptr) return;

            m_SwapChain = std::move(swapChain);
        }

        inline RefCntAutoPtr<ISwapChain>& GetSwapChain()
        {
            return m_SwapChain;
        }
    private:
        static void OnResizeCallback(GLFWwindow* rawWindow,int width,int height);
        static void OnFocusCallback(GLFWwindow* rawWindow, int focused);
        static void OnKeyCallback(GLFWwindow* rawWindow, int key, int scancode, int action, int mods);
        static void OnCursorPosCallBack(GLFWwindow* rawWindow, double x, double y);
        static void OnMouseScrollCallback(GLFWwindow* rawWindow, double xOffset, double yOffset);
        static void OnMouseButtonCallback(GLFWwindow* rawWindow, int button, int action, int mods);
    };
    DEFINE_PTR(FWindow)
}