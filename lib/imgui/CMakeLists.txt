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

if(BUILD_SHARED_LIBS)
    add_library(ImGui SHARED ${SOURCE_FILES})
else()
    add_library(ImGui STATIC ${SOURCE_FILES})
endif()


target_include_directories(ImGui PUBLIC imgui)
target_link_libraries(ImGui glfw glad)

if(WIN32)
    target_link_libraries(ImGui gdi32 opengl32 imm32)
endif()