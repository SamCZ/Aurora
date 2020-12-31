#pragma once

#include <GLFW/glfw3.h>

#include <Aurora/Core/String.hpp>
#include <Aurora/Core/SmartPointer.hpp>
#include <Aurora/Core/Sizeable.hpp>
#include "CursorMode.hpp"

namespace Aurora::App
{
    class FInputManager;
    PREDEFINE_PTR(class, FInputManager)

    struct FWindowDefinition
    {
        int Width;
        int Height;
        int XPositionOnScreen;
        int YPositionOnScreen;
        bool CenterScreen;

        bool HasOSWindowBorder;
        bool Maximized;

        String Title;
    };

    class FWindow : public FSizeable
    {
    private:
        static bool IS_GLFW_CONTEXT_INITIALIZED;
    private:
        GLFWwindow* m_WindowHandle;
        bool m_Focused;
        ECursorMode m_CursorMode;
    private:
        FInputManagerPtr m_InputManager;
    public:
        FWindow();

        void Initialize(FWindowDefinition& windowDefinition, const SharedPtr<FWindow>& parentWindow);

        void Show();
        void Hide();

        void Destroy();

        void Minimize();
        void Maximize();
        void Restore();

        void Focus();

        void SetTitle(const String& title);

        [[nodiscard]] bool IsFocused() const;

        [[nodiscard]] bool IsShouldClose() const;

        virtual GLFWwindow* GetWindowHandle();

        void SetCursorMode(const ECursorMode& mode);
        [[nodiscard]] const ECursorMode& GetCursorMode() const;

        bool IsIconified();

        FInputManagerPtr GetInputManager();
    private:
        static void OnResizeCallback(GLFWwindow* rawWindow,int width,int height);
        static void OnFocusCallback(GLFWwindow* rawWindow, int focused);
        static void OnKeyCallback(GLFWwindow* rawWindow, int key, int scancode, int action, int mods);
        static void OnCursorPosCallBack(GLFWwindow* rawWindow, double x, double y);
        static void OnMouseScrollCallback(GLFWwindow* rawWindow, double xOffset, double yOffset);
        static void OnMouseButtonCallback(GLFWwindow* rawWindow, int button, int action, int mods);
    };
}