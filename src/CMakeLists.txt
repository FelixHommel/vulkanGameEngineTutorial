set(NAME "vulkan")

add_executable(${NAME} main.cpp core/Window.cpp core/Application.cpp core/Pipeline.cpp core/Device.cpp #[[ core/Swapchain.cpp ]])
target_sources(${NAME} PRIVATE core/Window.hpp core/Application.hpp core/Pipeline.hpp core/Device.hpp #[[ core/Swapchain.hpp ]])
add_definitions(-DDEBUG)

target_link_libraries(${NAME} Vulkan::Vulkan glfw glm)

target_compile_options(${NAME} PRIVATE -g -O0 -fsanitize=address)
target_link_options(${NAME} PRIVATE -fsanitize=address)
