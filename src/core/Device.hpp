#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <vulkan/vulkan_core.h>

#include "Window.hpp"

#include <cstdint>
#include <optional>
#include <vector>

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

class Device
{
public:
    #ifdef DEBUG
        static constexpr bool enableValidationLayers{ true };
    #else
        static constexpr bool enableValidationLayers{ false };
    #endif

    Device(Window& window);
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;
    Device(Device&&) = delete;
    Device& operator=(Device&&) = delete;

    VkCommandPool getCommandPool() { return m_commandPool; }
    VkDevice device() { return m_device; }
    VkSurfaceKHR surface() { return m_surface; }
    VkQueue graphicsQueue() { return m_graphicsQueue; }
    VkQueue presentQueue() { return m_presentQueue; }

    SwapchainSupportDetails getSwapchainSupport() { return querySwapChainSupport(m_physicalDevice); }
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(m_physicalDevice); }
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    VkCommandBuffer beginSingleTimeCommand();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

    void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    VkPhysicalDeviceProperties properties;

private:
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    VkPhysicalDevice m_physicalDevice;
    Window& m_window;
    VkCommandPool m_commandPool;

    VkDevice m_device;
    VkSurfaceKHR m_surface;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;

    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    bool isDeviceSuitable(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void hasGlfwRequiredInstanceExtensions();
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapchainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    const std::vector<const char*> validationLayers{ "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};

#endif //!DEVICE_HPP
