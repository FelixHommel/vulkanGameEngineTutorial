#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include "Device.hpp"

#include <string>
#include <vector>

struct PipelineConfigInfo
{
};

class Pipeline
{
public:
    Pipeline(Device& device, const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
    ~Pipeline() = default;

    Pipeline(const Pipeline&) = delete;
    void operator=(const Pipeline&) = delete;

    static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

private:
    Device& device;
    VkPipeline m_graphicsPipeline;
    VkShaderModule m_vertShaderModule;
    VkShaderModule m_fragShaderModule;

    static std::vector<char> readFile(const std::string& filepath);

    void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
};

#endif //!PIPELINE_HPP
