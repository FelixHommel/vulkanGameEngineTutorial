cmake_minimum_required(VERSION 3.31)
project(vulkan LANGUAGES CXX)

find_package(Vulkan REQUIRED)

add_subdirectory(deps)

add_executable(${PROJECT_NAME} main.cpp Window.cpp Application.cpp Pipeline.cpp Device.cpp)
target_sources(${PROJECT_NAME} PRIVATE Window.hpp Application.hpp Pipeline.hpp Device.hpp)

target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan glfw glm)

target_compile_options(${PROJECT_NAME} PRIVATE -g -O0 -fsanitize=address)
target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
