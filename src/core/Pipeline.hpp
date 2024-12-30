#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include "Device.hpp"

#include <filesystem>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

struct PipelineConfigInfo
{
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    VkPipelineLayout pipelineLayout{ nullptr };
    VkRenderPass renderPass{ nullptr };
    uint32_t subpass{ 0 };
};

class Pipeline
{
public:
    Pipeline(Device& device, const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    void operator=(const Pipeline&) = delete;

    static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

    void bind(VkCommandBuffer commandBuffer);

private:
    Device& device;
    VkPipeline m_graphicsPipeline;
    VkShaderModule m_vertShaderModule;
    VkShaderModule m_fragShaderModule;

    static std::vector<char> readFile(const std::filesystem::path& filepath);

    void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
};

#endif //!PIPELINE_HPP
