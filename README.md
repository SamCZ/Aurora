# Aurora
### Free open source C++ game engine

## Supported Platforms
**Desktop**

| Name | Compiler | 32-bit | 64-bit | Render Context |
|------|:--------:|:------:|:------:|:--------------:|
| [Windows 10](https://en.wikipedia.org/wiki/Windows_10) | [MSVC](https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B) |  - | YES | OpenGL |
| [Windows 10](https://en.wikipedia.org/wiki/Windows_10) | [MinGW-w64](http://mingw-w64.org/) |  - | YES | OpenGL |
| [Windows 10](https://en.wikipedia.org/wiki/Windows_10) | [Cygwin](https://www.cygwin.com/) |  - | - | - |
| [Debian](https://www.debian.org/) / [Ubuntu](https://ubuntu.com/) | [GCC](https://gcc.gnu.org/) | - | In Development | OpenGL |
| [Debian](https://www.debian.org/) / [Ubuntu](https://ubuntu.com/) | [Clang](https://clang.llvm.org/) | - | - | - |
| [macOS](https://en.wikipedia.org/wiki/MacOS) | - | - | - | ~~Metal~~ |

## Used libraries
| Name | License | Version |
|------|---------|---------|
| [GLFW](https://www.glfw.org/) | [zlib/libpng](https://www.glfw.org/license.html) | branch: [`master`](https://github.com/glfw/glfw/tree/master) | 
| [GLM](https://glm.g-truc.net) | [MIT](https://glm.g-truc.net/copying.txt) | branch: [`master`](https://github.com/g-truc/glm/tree/master) |
| [GLAD](https://github.com/Dav1dde/glad) ([web](https://glad.dav1d.de/)) | [MIT](https://github.com/Dav1dde/glad/blob/master/LICENSE) | `OpenGL 4.6` |
| [ImGui](https://github.com/ocornut/imgui) | [MIT](https://github.com/ocornut/imgui/blob/docking/LICENSE.txt) | branch: [`master`]
| [nlohmann JSON](https://nlohmann.github.io/json/) | [MIT](https://github.com/nlohmann/json/blob/master/LICENSE.MIT) | branch: [`master`](https://github.com/nlohmann/json/tree/master) |
| [newton-dynamics](https://github.com/MADEAPPS/newton-dynamics) | [Newton zlib license](https://github.com/MADEAPPS/newton-dynamics/blob/master/LICENSE) | branch: [`master`](https://github.com/MADEAPPS/newton-dynamics/tree/master) |
| [FMOD](https://fmod.com/) | [Proprietary](https://fmod.com/legal) | [2.01.09 (build 115889)](https://fmod.com/download) |
| [entt](https://github.com/skypjack/entt) | [MIT License](https://github.com/skypjack/entt/blob/master/LICENSE) | branch: [`master`] |
| [freetype](https://www.freetype.org/) | [BSD-style](https://www.freetype.org/license.html) | local |
| [glslang](https://github.com/KhronosGroup/glslang) | [3-Clause BSD License](https://github.com/KhronosGroup/glslang/blob/master/LICENSE.txt) | branch: [`master`] |
| [RmlUI](https://github.com/mikke89/RmlUi) | [MIT License](https://github.com/mikke89/RmlUi/blob/master/LICENSE.txt) | branch: [`master`] |
| [tinygltf](https://github.com/syoyo/tinygltf) | [MIT License](https://github.com/syoyo/tinygltf/blob/master/LICENSE) | branch: [`master`] |
| [tracy](https://github.com/wolfpld/tracy) | [3-clause BSD license](https://github.com/wolfpld/tracy/blob/master/LICENSE) | branch: [`master`] |

## Installation

### Msys2 - for windows building
- Install [msys2](https://www.msys2.org/)
- Update msys2 package list (`pacman -Syuu`) (two times, first time it will close the window and you need to run it again)
- Install GCC `pacman -S mingw-w64-x86_64-gcc`
- Install GDB for debugging `pacman -S mingw-w64-x86_64-gdb`
- Install cmake `pacman -S mingw-w64-x86_64-cmake`
- Install make `pacman -S mingw-w64-x86_64-make`
- Add MinGW to your IDE (`C:\msys64\mingw64`)

### CMake options
| Name | Value info | Default | Info |
|------|---------|---------|---------|
| FMOD_API_DIR | Path to installed fmod api folder. | `C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api` | FMod folder for sound support |
| BUILD_EXAMPLES | ON / OFF | `OFF` | This will enable / disable examples build |

### OpenGL problems that I found

#### On AMD GPU
 - GLSL will throw error `#endif not found` if use `#ifdef` even if `#endif` exists.
 - Ternary operators on `sampler2D` maybe others in fragments shaders will throw very weird error while linking vertex and fragment shader `INVALID_OPERATION`, example: `texture(Side == 2 ? TextureTop : TextureSide, TexCoord)`
