# `heatsink`
C++-style utility library for OpenGL.

Templatized classes for the basic OpenGL object types. Still a work in progress; framebuffers, textures, etc. coming soon!

Requires C++20 features - build with CMake and the following libraries: `glfw3`, `libepoxy`, and `glm`.
* Depending on compiler, you may also need to add the C++ filesystem library to the CMake libs.
* Current coolest feature uses templates to deduce buffer parameters for any scalar-like objects. See `traits/tensor.hpp` and `gl/buffer.hpp`.
