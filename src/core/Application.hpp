#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "Device.hpp"
#include "Window.hpp"
#include "Pipeline.hpp"

class Application
{
public:
    static constexpr int WIDTH{ 800 };
    static constexpr int HEIGHT{ 600 };

    void run();

private:
    Window m_window{ WIDTH, HEIGHT, "vulkan" };
    Device m_device{ m_window };
    Pipeline m_pipeline{ m_device, "shaders/simple.vert.spv", "shaders/simple.frag.spv", Pipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT) };
};

#endif //!APPLICATION_HPP
