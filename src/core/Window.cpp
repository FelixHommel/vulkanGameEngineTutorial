#include "Window.hpp"

#include <GLFW/glfw3.h>
#include <stdexcept>

Window::Window(int width, int height, const std::string& title)
    : m_width(width), m_height(height), m_title(title)
{
    if(!glfwInit())
        throw std::runtime_error("GLFW Failure while initializing GLFW");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if(!m_window)
        throw std::runtime_error("GLFW Failure while creating the window");
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Window::createWindowSurface(VkInstance& instance, VkSurfaceKHR* surface)
{
    if(glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS)
        throw std::runtime_error("GLFW Failure while Creating Window Surface");
}
