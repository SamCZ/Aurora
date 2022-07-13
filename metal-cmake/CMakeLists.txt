# Library definition
add_library(METAL_CPP
        defination.cpp
        )

# Metal cpp headers
target_include_directories(METAL_CPP PUBLIC
        "metal-cpp"
        "metal-cpp-extensions"
        )

# Metal cpp library (linker)
target_link_libraries(METAL_CPP
        "-framework Metal"
        "-framework MetalKit"
        "-framework AppKit"
        "-framework Foundation"
        "-framework QuartzCore"
        )
