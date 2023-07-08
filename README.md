# SFML with IGL

Using the Simple and Fast Multimedia Library (SFML) for windowing and input,
and the Intermediate Graphics Library (IGL) for graphics rendering.

This example renders a triangle using the Vulkan graphics API.

![Vulkan Triangle](https://github.com/eXpl0it3r/SFML-IGL/assets/920861/6f79044a-38d9-4c57-a67f-fa53ce9ee3f5)

## Pre-Requisites

-   Vulkan SDK
    -   https://vulkan.lunarg.com/
    -   Windows: `winget install KhronosGroup.VulkanSDK`
-   Python 3
    -   https://www.python.org/downloads/
    -   Windows: `winget install Python -s winget`
-   CMake
    -   https://cmake.org/download/
    -   Windows: `winget install Kitware.CMake`
-   Compiler with C++20 Support
    -   Requires `<format>`

## Building

-   `cmake -S . -B build`
-   `cmake --build build`

## Resources

-   https://www.sfml-dev.org
-   https://github.com/SFML/SFML
-   https://github.com/facebook/igl

## License

As the example is heavily based on the GLFW example provided as part of IGL,
the same license has been adopted: MIT License

-   SFML - [zlib License](https://github.com/SFML/SFML/blob/master/license.md)
-   IGL - [MIT License](https://github.com/facebook/igl/blob/main/LICENSE.md)
