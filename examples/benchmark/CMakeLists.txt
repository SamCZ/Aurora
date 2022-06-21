project(Benchmarks C CXX)

add_executable(memory_benchmark memory_benchmark.cpp)
target_link_libraries(memory_benchmark Aurora)