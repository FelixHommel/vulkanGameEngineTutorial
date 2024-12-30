#ifndef CORE_SWAPCHAIN_HPP
#define CORE_SWAPCHAIN_HPP

#include "Device.hpp"

#include <vulkan/vulkan_core.h>

#include <vector>

class Swapchain
{
public:
    static constexpr unsigned int MAX_FRAMES_IN_FLIGHT{ 2 };

    Swapchain(Device& device, VkExtent2D windowExtent);
    ~Swapchain();

    Swapchain(const Swapchain&) = delete;
    void operator=(const Swapchain&) = delete;

    VkFramebuffer getFramebuffer(size_t index) { return m_swapchainFramebuffers.at(index); }
    VkRenderPass getRenderPass() { return m_renderPass; }
    VkImageView getImageView(size_t index) { return m_swapchainImageViews.at(index); }
    size_t imageCount() { return m_swapchainImages.size(); }
    VkFormat getSwapchainImageFormat() { return m_swapchainImageFormat; }
    VkExtent2D getSwapchainExtent() { return m_swapchainExtent; }
    uint32_t width() { return m_swapchainExtent.width; }
    uint32_t height() { return m_swapchainExtent.height; }

    float extentAspectRatio() { return static_cast<float>(m_swapchainExtent.width) / static_cast<float>(m_swapchainExtent.height); }
    VkFormat findDepthFormat();

    VkResult acquireNextImage(uint32_t* imageIndex);
    VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

private:
    VkFormat m_swapchainImageFormat;
    VkExtent2D m_swapchainExtent;

    std::vector<VkFramebuffer> m_swapchainFramebuffers;
    VkRenderPass m_renderPass;

    std::vector<VkImage> m_depthImages;
    std::vector<VkDeviceMemory> m_depthImageMemories;
    std::vector<VkImageView> m_depthImageViews;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;

    Device& device;
    VkExtent2D m_windowExtent;

    VkSwapchainKHR m_swapchain;

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
