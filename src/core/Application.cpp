#include "Application.hpp"

#include <array>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

Application::Application()
    : m_window{ WIDTH, HEIGHT, "vulkan" }
    , m_device{ m_window }
    , m_swapchain{ m_device, m_window.getExtent() }
{
    createPipelineLayout();
    createPipeline();
    createCommandBuffers();
}

Application::~Application()
{
    vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
}

void Application::run()
{
    while(!m_window.shouldClose())
    {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(m_device.device());
}

void Application::createPipelineLayout()
{
    VkPipelineLayoutCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };

    if(vkCreatePipelineLayout(m_device.device(), &createInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failure while creating pipeline layout");
}

void Application::createPipeline()
{
    auto pipelineConfig{ Pipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT) };
    pipelineConfig.renderPass = m_swapchain.getRenderPass();
    pipelineConfig.pipelineLayout = m_pipelineLayout;

    m_pipeline = std::make_unique<Pipeline>(m_device, "shaders/simple.vert.spv", "shaders/simple.frag.spv", pipelineConfig);
}

void Application::createCommandBuffers()
{
    m_commandBuffers.resize(m_swapchain.imageCount());

    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_device.getCommandPool(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size())
    };

    if(vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failure while allocating command buffers");

    for(size_t i{ 0 }; i < m_commandBuffers.size(); ++i)
    {
        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        };

        if(vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("Failure while begining to record command buffer");

        std::array<VkClearValue, 2> clearValues{ VkClearValue{ .color = { 0.1f, 0.1f, 0.1f, 1.f } }, VkClearValue{ .depthStencil = { 1.f, 0} } };
        VkRenderPassBeginInfo renderPassInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = m_swapchain.getRenderPass(),
            .framebuffer = m_swapchain.getFramebuffer(i),
            .renderArea = VkRect2D{
                .offset = { 0, 0 },
                .extent = m_swapchain.getSwapchainExtent()
            },
            .clearValueCount = static_cast<uint32_t>(clearValues.size()),
            .pClearValues = clearValues.data()
        };

        vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        m_pipeline->bind(m_commandBuffers[i]);
        vkCmdDraw(m_commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(m_commandBuffers[i]);

        if(vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Failure while recording command buffer");
    }
}

void Application::drawFrame()
{
    uint32_t imageIndex;
    auto result{ m_swapchain.acquireNextImage(&imageIndex) };

    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failure while acquireing swap chain image");

    result = m_swapchain.submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);

    if(result != VK_SUCCESS)
        throw std::runtime_error("failure while submitting command buffer");
}
