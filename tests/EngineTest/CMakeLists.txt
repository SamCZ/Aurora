add_executable(EngineTest main.cpp)
target_link_libraries(EngineTest AuroraEngine)
add_test(NAME EngineTest COMMAND EngineTest)

target_compile_definitions(EngineTest PRIVATE TEST_PROJECT_BINARY_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}")