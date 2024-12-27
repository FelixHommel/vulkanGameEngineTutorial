#ifndef CORE_SWAPCHAIN_HPP
#define CORE_SWAPCHAIN_HPP

#include "Device.hpp"

#include <vulkan/vulkan_core.h>

#include <vector>

class Swapchain
{
public:
    static constexpr unsigned int MAX_FRAMES_IN_FLIGHT{ 2 };

    Swapchain(Device& deviceRef, VkExtent2D windowExtent);
    ~Swapchain();

    Swapchain(const Swapchain&) = delete;
    void operator=(const Swapchain&) = delete;

    VkFramebuffer getFramebuffer(size_t index) { return m_swapChainFramebuffers.at(index); }
    VkRenderPass getRenderPass() { return m_renderPass; }
    VkImageView getImageView(size_t index) { return m_swapChainImageViews.at(index); }
    size_t imageCount() { return m_swapChainImages.size(); }
    VkFormat getSwapChainFormat() { return m_swapChainImageFormat; }
    VkExtent2D getSwapChainExtent() { return m_swapChainExtent; }
    uint32_t width() { return m_swapChainExtent.width; }
    uint32_t height() { return m_swapChainExtent.height; }

    float extentAspectRatio() { return static_cast<float>(m_swapChainExtent.width) / static_cast<float>(m_swapChainExtent.height); }
    VkFormat findDepthFormat();

    VkResult acquireNextImage(uint32_t* imageIndex);
    VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

private:
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;

    std::vector<VkFramebuffer> m_swapChainFramebuffers;
    VkRenderPass m_renderPass;

    std::vector<VkImage> m_depthImages;
    std::vector<VkDeviceMemory> m_depthImageMemories;
    std::vector<VkImageView> m_depthImageViews;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;

    Device& device;
    VkExtent2D m_windowExtent;

    VkSwapchainKHR m_swapChain;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;
    size_t m_currentFrame{ 0 };

    void createSwapchain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

    VkSurfaceFormatKHR chooseSwapSurfcaeFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};

#endif //!CORE_SWAPCHAIN_HPP
