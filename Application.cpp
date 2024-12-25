#include "Application.hpp"

void Application::run()
{
    while(!m_window.shouldClose())
    {
        glfwPollEvents();
    }
}
