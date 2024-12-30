set(NAME "vulkan")

if(UNIX)
    set(glslc_executable "/usr/local/bin/glslc")
elseif(WIN32)
    if(DEFINED ENV{VULKAN_SDK})
        set(glslc_executable "$ENV{VULKAN_SDK}/Bin/glslc.exe")
    else()
        message(FATAL_ERROR "could not find glslc location")
    endif()
endif()

if(NOT EXISTS ${glslc_executable})
    message(FATAL_ERROR "glslc executable not found")
endif()

set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_SOURCE_DIR}/shaders)

file(GLOB SHADERS
    ${CMAKE_SOURCE_DIR}/shaders/simple.vert
    ${CMAKE_SOURCE_DIR}/shaders/simple.frag
)

add_custom_command(
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
    OUTPUT ${SHADER_BINARY_DIR}
    COMMENT "Creating ${SHADER_BINARY_DIR}"
)

foreach(source IN LISTS SHADERS)
    get_filename_component(FILENAME ${source} NAME)
    add_custom_command(
        COMMAND
            ${glslc_executable}
            -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
            ${source}
        OUTPUT ${SHADER_BIANRY_DIR}/${FILENAME}.spv
        DEPENDS ${source} ${SHADER_BINARY_DIR}
        COMMENT "Compiling ${FILENAME}"
    )

    list(APPEND SPV_SHADERS ${SHADER_BIANRY_DIR}/${FILENAME}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
message(STATUS "Compiled all shaders")

find_program(GLSLC  NAMES glslc REQUIIRED)
add_executable(${NAME} main.cpp core/Window.cpp core/Application.cpp core/Pipeline.cpp core/Device.cpp core/Swapchain.cpp)
target_sources(${NAME} PRIVATE core/Window.hpp core/Application.hpp core/Pipeline.hpp core/Device.hpp core/Swapchain.hpp)
add_definitions(-DDEBUG)

target_link_libraries(${NAME} Vulkan::Vulkan glfw glm)

target_compile_options(${NAME} PRIVATE -g -O0 -fsanitize=address)
target_link_options(${NAME} PRIVATE -fsanitize=address)
