#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <vulkan/vulkan_core.h>

#include "Window.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"

#include <memory>
#include <vector>

class Application
{
public:
    static constexpr int WIDTH{ 800 };
    static constexpr int HEIGHT{ 600 };

    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void run();

private:
    Window m_window;
    Device m_device;
    Swapchain m_swapchain;
    std::unique_ptr<Pipeline> m_pipeline;

    VkPipelineLayout m_pipelineLayout;
    std::vector<VkCommandBuffer> m_commandBuffers;

    void createPipelineLayout();
    void createPipeline();
    void createCommandBuffers();

    void drawFrame();
};

#endif //!APPLICATION_HPP
