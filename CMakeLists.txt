cmake_minimum_required(VERSION 3.20)
project(UvkornioVisualizer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

add_executable(uvkornio_visualizer
    src/main.cpp
    src/audio_stream.cpp
    src/spectrum_analyzer.cpp
    src/surround_analyzer.cpp
    src/vulkan_context.cpp
    src/visualizer.cpp
    src/waterfall_renderer.cpp
)

target_include_directories(uvkornio_visualizer PRIVATE src)
target_link_libraries(uvkornio_visualizer PRIVATE Vulkan::Vulkan)
