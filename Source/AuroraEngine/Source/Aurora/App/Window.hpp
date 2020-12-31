#pragma once

#include <GLFW/glfw3.h>

#include <Aurora/Core/String.hpp>

namespace Aurora::App
{
    class FInputManager;

    struct FWindowDefinition
    {
        int Width;
        int Height;
        int XPositionOnScreen;
        int YPositionOnScreen;
        bool CenterScreen;

        bool HasOSWindowBorder;

        String Title;
    };

    class FWindow
    {
    private:
        GLFWwindow* m_WindowHandle;
    public:
        void Initialize();

    private:
        static void OnResizeCallback(GLFWwindow* rawWindow,int width,int height);
        static void OnFocusCallback(GLFWwindow* rawWindow, int focused);
        static void OnKeyCallback(GLFWwindow* rawWindow, int key, int scancode, int action, int mods);
        static void OnCursorPosCallBack(GLFWwindow* rawWindow, double x, double y);
        static void OnMouseScrollCallback(GLFWwindow* rawWindow, double xOffset, double yOffset);
        static void OnMouseButtonCallback(GLFWwindow* rawWindow, int button, int action, int mods);
    };
}