project(ImGui C CXX)

set(SOURCE_FILES
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_opengl3.cpp
        )

add_library(ImGui STATIC ${SOURCE_FILES})
target_include_directories(ImGui PUBLIC imgui)
target_link_libraries(ImGui glfw glad gdi32 opengl32 imm32)