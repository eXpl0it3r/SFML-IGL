cmake_minimum_required(VERSION 3.21)
project(SFML-IGL LANGUAGES CXX)

SET(BUILD_SHARED_LIBS OFF)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG master)
FetchContent_MakeAvailable(SFML)

SET(IGL_WITH_IGLU OFF)
SET(IGL_WITH_SHELL OFF)
SET(IGL_WITH_TESTS OFF)
SET(IGL_WITH_TRACY OFF)
SET(IGL_WITH_SAMPLES OFF)

add_definitions("-DIGL_CMAKE_BUILD=1")

FetchContent_Declare(IGL
    GIT_REPOSITORY https://github.com/facebook/igl.git
    GIT_TAG main)
FetchContent_MakeAvailable(IGL)

add_executable(TriangleExample main.cpp)
target_link_libraries(TriangleExample PRIVATE sfml-window IGLLibrary)
target_compile_features(TriangleExample PRIVATE cxx_std_20)
if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET TriangleExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:TriangleExample> $<TARGET_FILE_DIR:TriangleExample> COMMAND_EXPAND_LISTS)
endif()

install(TARGETS TriangleExample)