cmake_minimum_required(VERSION 3.28)
project(SFML-IGL LANGUAGES CXX)

option(BUILD_SHARED_LIBS OFF)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG master
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

option(IGL_WITH_IGLU OFF)
option(IGL_WITH_SHELL OFF)
option(IGL_WITH_TESTS OFF)
option(IGL_WITH_TRACY OFF)
option(IGL_WITH_SAMPLES OFF)

add_definitions("-DIGL_CMAKE_BUILD=1")

FetchContent_Declare(IGL
    GIT_REPOSITORY https://github.com/facebook/igl.git
    GIT_TAG main
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(IGL)

add_executable(TriangleExample main.cpp)
target_compile_features(TriangleExample PRIVATE cxx_std_20)
target_link_libraries(TriangleExample PRIVATE SFML::Window IGLLibrary)

if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET TriangleExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:TriangleExample> $<TARGET_FILE_DIR:TriangleExample> COMMAND_EXPAND_LISTS)
endif()

install(TARGETS TriangleExample)