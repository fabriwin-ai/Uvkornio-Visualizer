cmake_minimum_required(VERSION 3.20)
project(UvkornioVisualizer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)
find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    include(FetchContent)
    FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.9
    )
    set(GLFW_BUILD_EXAMPLES OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_BUILD_DOCS OFF)
    set(GLFW_INSTALL OFF)
    FetchContent_MakeAvailable(glfw)
endif()
find_package(ALSA QUIET)

add_executable(uvkornio_visualizer
    src/main.cpp
    src/audio_stream.cpp
    src/differential_math.cpp
    src/microphone_input.cpp
    src/spectrum_analyzer.cpp
    src/surround_analyzer.cpp
    src/vulkan_app.cpp
    src/vulkan_context.cpp
    src/visualizer.cpp
    src/waterfall_renderer.cpp
)

target_include_directories(uvkornio_visualizer PRIVATE src)
target_link_libraries(uvkornio_visualizer PRIVATE Vulkan::Vulkan glfw)

if(ALSA_FOUND)
    target_compile_definitions(uvkornio_visualizer PRIVATE UVK_ENABLE_ALSA)
    target_link_libraries(uvkornio_visualizer PRIVATE ALSA::ALSA)
endif()
