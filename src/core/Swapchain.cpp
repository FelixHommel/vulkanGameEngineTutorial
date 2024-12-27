#include "Swapchain.hpp"

#include <vulkan/vulkan_core.h>

#include <array>
#include <cstdint>
#include <stdexcept>

Swapchain::Swapchain(Device& deviceRef, VkExtent2D extent)
    : device(deviceRef), m_windowExtent(extent)
{
    createSwapchain();
    createImageViews();
    createRenderPass();
    createDepthResources();
    createFramebuffers();
    createSyncObjects();
}

Swapchain::~Swapchain()
{
    for(auto imageView: m_swapChainImageViews)
        vkDestroyImageView(device.device(), imageView, nullptr);
    m_swapChainImageViews.clear();

    if(m_swapChain != nullptr)
    {
        vkDestroySwapchainKHR(device.device(), m_swapChain, nullptr);
        m_swapChain = nullptr;
    }

    for(size_t i{ 0 }; i < m_depthImages.size(); ++i)
    {
        vkDestroyImageView(device.device(), m_depthImageViews[i], nullptr);
        vkDestroyImage(device.device(), m_depthImages[i], nullptr);
        vkFreeMemory(device.device(), m_depthImageMemories[i], nullptr);
    }

    for(auto framebuffer: m_swapChainFramebuffers)
        vkDestroyFramebuffer(device.device(), framebuffer, nullptr);

    vkDestroyRenderPass(device.device(), m_renderPass, nullptr);

    for(size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(device.device(), m_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device.device(), m_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device.device(), m_inFlightFences[i], nullptr);
    }
}

VkResult Swapchain::acquireNextImage(uint32_t* imageIndex)
{
    vkWaitForFences(device.device(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    return vkAcquireNextImageKHR(device.device(), m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, imageIndex);
}

VkResult Swapchain::submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex)
{
    if(m_imagesInFlight[*imageIndex] != VK_NULL_HANDLE)
        vkWaitForFences(device.device(), 1, &m_imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
    m_imagesInFlight[*imageIndex] = m_inFlightFences[m_currentFrame];


    std::array<VkSemaphore, 1> waitSemaphores{ m_imageAvailableSemaphores[m_currentFrame] };
    std::array<VkPipelineStageFlags, 1> waitStages{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    std::array<VkSemaphore, 1> signalSemaphores{ m_renderFinishedSemaphores[m_currentFrame] };
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = waitStages.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = buffers,
        .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .pSignalSemaphores = signalSemaphores.data()
    };

    vkResetFences(device.device(), 1, &m_inFlightFences[m_currentFrame]);
    if(vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("Failure while submitting draw command buffer");

    std::array<VkSwapchainKHR, 1> swapchains{ m_swapChain };
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .pWaitSemaphores = signalSemaphores.data(),
        .swapchainCount = static_cast<uint32_t>(swapchains.size()),
        .pSwapchains = swapchains.data(),
        .pImageIndices = imageIndex,
    };

    auto result{ vkQueuePresentKHR(device.graphicsQueue(), &presentInfo) };

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
}
