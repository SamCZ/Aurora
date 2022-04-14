# Aurora
### Free open source C++ game engine

**STILL WORK IN PROGRESS!**

![Engine screenshot](https://raw.githubusercontent.com/SamCZ/Aurora/main/screenshots/Screenshot-1.png)

## Supported Platforms
**Desktop**

| Name | Compiler | 32-bit | 64-bit | Render Context |
|------|:--------:|:------:|:------:|:--------------:|
| [Windows 10](https://en.wikipedia.org/wiki/Windows_10) | [MSVC](https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B) |  - |  YES   | OpenGL |
| [Windows 10](https://en.wikipedia.org/wiki/Windows_10) | [MinGW-w64](http://mingw-w64.org/) |  - |  YES   | OpenGL |
| [Windows 10](https://en.wikipedia.org/wiki/Windows_10) | [Cygwin](https://www.cygwin.com/) |  - |   -    | - |
| [Debian](https://www.debian.org/) / [Ubuntu](https://ubuntu.com/) | [GCC](https://gcc.gnu.org/) | - |  YES   | OpenGL |
| [Debian](https://www.debian.org/) / [Ubuntu](https://ubuntu.com/) | [Clang](https://clang.llvm.org/) | - |   -    | - |
| [macOS](https://en.wikipedia.org/wiki/MacOS) | - | - |   -    | ~~Metal~~ |

## Used libraries
| Name                                                                    | License                                                                                 | Version |
|-------------------------------------------------------------------------|-----------------------------------------------------------------------------------------|---------|
| [GLFW](https://www.glfw.org/)                                           | [zlib/libpng](https://www.glfw.org/license.html)                                        | branch: [`master`](https://github.com/glfw/glfw/tree/master) | 
| [GLM](https://glm.g-truc.net)                                           | [MIT](https://glm.g-truc.net/copying.txt)                                               | branch: [`master`](https://github.com/g-truc/glm/tree/master) |
| [GLAD](https://github.com/Dav1dde/glad) ([web](https://glad.dav1d.de/)) | [MIT](https://github.com/Dav1dde/glad/blob/master/LICENSE)                              | `OpenGL 4.6` |
| [ImGui](https://github.com/ocornut/imgui)                               | [MIT](https://github.com/ocornut/imgui/blob/docking/LICENSE.txt)                        | branch: [`docking`]
| [nlohmann JSON](https://nlohmann.github.io/json/)                       | [MIT](https://github.com/nlohmann/json/blob/master/LICENSE.MIT)                         | branch: [`master`](https://github.com/nlohmann/json/tree/master) |
| [FMOD](https://fmod.com/)                                               | [Proprietary](https://fmod.com/legal)                                                   | [2.01.09 (build 115889)](https://fmod.com/download) |
| [freetype](https://www.freetype.org/)                                   | [BSD-style](https://www.freetype.org/license.html)                                      | local |
| [RmlUI](https://github.com/mikke89/RmlUi)                               | [MIT License](https://github.com/mikke89/RmlUi/blob/master/LICENSE.txt)                 | branch: [`master`] |
| [Assimp](https://github.com/syoyo/tinygltf)                             | [Poly2Tri License](https://github.com/assimp/assimp/blob/master/LICENSE)               | branch: [`master`] |
| [tracy](https://github.com/wolfpld/tracy)                               | [3-clause BSD license](https://github.com/wolfpld/tracy/blob/master/LICENSE)            | branch: [`master`] |

## Installation

### MSVC
 - Install [cmake-gui](https://cmake.org/download/)
 - Start cmake-gui
 - Open root project folder (Aurora)
 - Select build folder
 - Click configure
 - Click generate
 - Open VS project

### Msys2 - for windows gcc building
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
 - Ternary operators on `sampler2D` maybe others in fragments shaders will throw very weird error while linking vertex and fragment shader `INVALID_OPERATION`, example: `texture(Side == 2 ? TextureTop : TextureSide, TexCoord)`
