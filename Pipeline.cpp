#include "Pipeline.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <vulkan/vulkan_core.h>

Pipeline::Pipeline(Device& device, const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo)
    : device(device)
{
    auto vertBin{ readFile(vertFilepath) };
    auto fragBin{ readFile(fragFilepath) };

    std::cout << "Vertex shader size: " << vertBin.size() << '\n' << "Fragment shader size: " << fragBin.size() << std::endl;
}

PipelineConfigInfo Pipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height)
{
    PipelineConfigInfo configInfo{
    };

    return configInfo;
}

std::vector<char> Pipeline::readFile(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if(!file.is_open())
        throw std::runtime_error("Failure opening the file at: " + filepath);

    size_t fileSize{ static_cast<size_t>(file.tellg()) };
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

void Pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
{
    VkShaderModuleCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data())
    };

    if(vkCreateShaderModule(device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
        throw std::runtime_error("Failure while Creating shader module");
}
