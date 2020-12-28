add_executable(RenderTest main.cpp)
target_link_libraries(RenderTest Aurora)
add_test(NAME RenderTest COMMAND Aurora)
copy_required_dlls(RenderTest)