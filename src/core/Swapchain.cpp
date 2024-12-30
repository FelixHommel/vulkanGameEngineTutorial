#include "Swapchain.hpp"
#include "Device.hpp"

#include <algorithm>
#include <iostream>
#include <vulkan/vulkan_core.h>

#include <array>
#include <cstdint>
#include <stdexcept>

Swapchain::Swapchain(Device& device, VkExtent2D extent)
    : device(device), m_windowExtent(extent)
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
    for(auto imageView: m_swapchainImageViews)
        vkDestroyImageView(device.device(), imageView, nullptr);
    m_swapchainImageViews.clear();

    if(m_swapchain != nullptr)
    {
        vkDestroySwapchainKHR(device.device(), m_swapchain, nullptr);
        m_swapchain = nullptr;
    }

    for(size_t i{ 0 }; i < m_depthImages.size(); ++i)
    {
        vkDestroyImageView(device.device(), m_depthImageViews[i], nullptr);
        vkDestroyImage(device.device(), m_depthImages[i], nullptr);
        vkFreeMemory(device.device(), m_depthImageMemories[i], nullptr);
    }

    for(auto framebuffer: m_swapchainFramebuffers)
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

    return vkAcquireNextImageKHR(device.device(), m_swapchain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, imageIndex);
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

    std::array<VkSwapchainKHR, 1> swapchains{ m_swapchain };
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

void Swapchain::createSwapchain()
{
    SwapchainSupportDetails swapchainSupport{ device.getSwapchainSupport() };

    VkSurfaceFormatKHR surfaceFormat{ chooseSwapSurfcaeFormat(swapchainSupport.formats) };
    VkPresentModeKHR presentMode{ chooseSwapPresentMode(swapchainSupport.presentModes) };
    VkExtent2D extent{ chooseSwapExtent(swapchainSupport.capabilities) };

    uint32_t imageCount{ swapchainSupport.capabilities.minImageCount + 1 };
    if(swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
        imageCount = swapchainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = device.surface(),
        .minImageCount = swapchainSupport.capabilities.minImageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = swapchainSupport.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    QueueFamilyIndices indices{ device.findPhysicalQueueFamilies() };
    uint32_t queueFamilyIndices[]{ indices.graphicsFamily.value(), indices.presentFamily.value() };

    if(indices.graphicsFamily.value() != indices.presentFamily.value())
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    if(vkCreateSwapchainKHR(device.device(), &createInfo, nullptr, &m_swapchain) != VK_SUCCESS)
        throw std::runtime_error("Failure while creating swapchain");

    vkGetSwapchainImagesKHR(device.device(), m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device.device(), m_swapchain, &imageCount, m_swapchainImages.data());
    
    m_swapchainImageFormat = surfaceFormat.format;
    m_swapchainExtent = extent;
}

void Swapchain::createImageViews()
{
    m_swapchainImageViews.resize(imageCount());

    for(size_t i{ 0 }; i < imageCount(); ++i)
    {
        VkImageViewCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_swapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_swapchainImageFormat,
            .subresourceRange = VkImageSubresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        if(vkCreateImageView(device.device(), &createInfo, nullptr, &m_swapchainImageViews[i]) != VK_SUCCESS)
            throw std::runtime_error("Failure while creating texture iamge views");
    }
}

void Swapchain::createRenderPass()
{
    VkAttachmentDescription depthAttachment{
        .format = findDepthFormat(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference depthAttachmentRef{
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkAttachmentDescription colorAttachment{
        .format = getSwapchainImageFormat(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentRef{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pDepthStencilAttachment = &depthAttachmentRef
    };

    VkSubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
    };

    std::array<VkAttachmentDescription, 2> attachments{ colorAttachment, depthAttachment };
    VkRenderPassCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };

    if(vkCreateRenderPass(device.device(), &createInfo, nullptr, &m_renderPass) != VK_SUCCESS)
        throw std::runtime_error("Failure while creating render pass");
}

void Swapchain::createFramebuffers()
{
    m_swapchainFramebuffers.resize(imageCount());

    for(size_t i{ 0 }; i < imageCount(); ++i)
    {
        std::array<VkImageView, 2> attachments{ m_swapchainImageViews[i], m_depthImageViews[i] };

        VkExtent2D swapchainExtent{ getSwapchainExtent() };
        VkFramebufferCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_renderPass,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = swapchainExtent.width,
            .height = swapchainExtent.height,
            .layers = 1
        };

        if(vkCreateFramebuffer(device.device(), &createInfo, nullptr, &m_swapchainFramebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Failure while creating framebuffers");
    }
}

void Swapchain::createDepthResources()
{
    VkFormat depthFormat{ findDepthFormat() };
    VkExtent2D swapchainExtent{ getSwapchainExtent() };

    m_depthImages.resize(imageCount());
    m_depthImageMemories.resize(imageCount());
    m_depthImageViews.resize(imageCount());

    for(size_t i{ 0 }; i < imageCount(); ++i)
    {
        VkImageCreateInfo imageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = depthFormat,
            .extent = VkExtent3D{
                .width = swapchainExtent.width,
                .height = swapchainExtent.height,
                .depth = 1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        device.createImageWithInfo(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImages[i], m_depthImageMemories[i]);

        VkImageViewCreateInfo imageViewCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_depthImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = depthFormat,
            .subresourceRange = VkImageSubresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        if(vkCreateImageView(device.device(), &imageViewCreateInfo, nullptr, &m_depthImageViews[i]) != VK_SUCCESS)
            throw std::runtime_error("Failure while creating depth image views");
    }
}

void Swapchain::createSyncObjects()
{
    m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    m_imagesInFlight.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fenceInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for(size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if(vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device.device(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failure while creating Sync objects");
        }
    }
}

VkSurfaceFormatKHR Swapchain::chooseSwapSurfcaeFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for(const auto& availableFormat: availableFormats)
    {
        if(availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;
    }

    return availableFormats[0];
}

VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for(const auto& presentMode: availablePresentModes)
    {
        if(presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            std::clog << "Present mode: Mailbox" << std::endl;
            return presentMode;
        }
    }

    std::clog << "Present mode: V-Sync" << std::endl;
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if(capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;
    else
    {
        VkExtent2D actualExtent{ m_windowExtent };
        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.minImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.minImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

VkFormat Swapchain::findDepthFormat()
{
    return device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
