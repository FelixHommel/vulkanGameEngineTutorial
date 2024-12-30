#ifndef WINDOW_HPP
#define WINDOW_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>

class Window
{ 
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    inline bool shouldClose() { return glfwWindowShouldClose(m_window); }
    inline VkExtent2D getExtent() { return { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height) }; }

    void createWindowSurface(VkInstance& instance, VkSurfaceKHR* surface);

private:
    GLFWwindow* m_window;
    const int m_width;
    const int m_height;
    const std::string m_title;

    void initWindow();
};

#endif // !WINDOW_HPP
