#include "ProceduralCompute.h"

#include <fstream>
#include <chrono>

namespace app {

std::vector<uint8_t> loadFile(const std::string &str)
{
	std::basic_ifstream<uint8_t> file(str, std::ios::binary | std::ios::ate);
	std::streampos size = file.tellg();
	file.seekg(0);
	std::vector<uint8_t> content(size);
	file.read(content.data(), content.size());
	return content;
}

VkShaderModule createShaderModule(const vk::Device &device, const std::string &filePath) {
	std::string buildPath = filePath + ".spv";
	char *env;
	size_t size;
	_dupenv_s(&env, &size, "VULKAN_SDK");
	if (size == 0)
		throw std::runtime_error("VULKAN_SDK not found");
	char buffer[256];
	snprintf(buffer, 256, "%s/Bin32/glslangValidator.exe -V %s -o %s", env, filePath.c_str(), buildPath.c_str());
	if (0 != system(buffer))
		throw std::runtime_error("Error building");
	std::vector<uint8_t> code = loadFile(buildPath);
	VkShaderModuleCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
	VkShaderModule shaderModule;
	VK_CHECK_RESULT(vkCreateShaderModule(device(), &createInfo, nullptr, &shaderModule));
	return shaderModule;
}

void ProceduralCompute::create(const vk::Device & device)
{
	// Descriptor set layout
	m_descriptorBindings.resize(1);

	m_descriptorBindings[0].binding = 0;
	m_descriptorBindings[0].descriptorCount = 1;
	m_descriptorBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	m_descriptorBindings[0].pImmutableSamplers = nullptr;
	m_descriptorBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(m_descriptorBindings.size());
	layoutInfo.pBindings = m_descriptorBindings.data();

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device(), &layoutInfo, nullptr, &m_descriptorSetLayout));

	// Descriptor pool
	std::vector<VkDescriptorPoolSize> poolSizes(m_descriptorBindings.size(), {});
	for (size_t i = 0; i < poolSizes.size(); i++)
	{
		poolSizes[i].type = m_descriptorBindings[i].descriptorType;
		poolSizes[i].descriptorCount = m_descriptorBindings[i].descriptorCount;
	}

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 1;

	VK_CHECK_RESULT(vkCreateDescriptorPool(device(), &poolInfo, nullptr, &m_descriptorPool));

	// Descriptor set
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &m_descriptorSetLayout;

	VK_CHECK_RESULT(vkAllocateDescriptorSets(device(), &allocInfo, &m_descriptorSet));

	// Pipeline
	VkShaderModule shaderModule = createShaderModule(device, "data/shaders/procedural.comp");

	VkPipelineShaderStageCreateInfo shaderStageInfo{};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shaderStageInfo.module = shaderModule;
	shaderStageInfo.pName = "main";

	VkPushConstantRange pushConstants{};
	pushConstants.offset = 0;
	pushConstants.size = sizeof(PushConstant);
	pushConstants.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	// Create the layout of the pipeline following the provided descriptor set layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstants;

	VK_CHECK_RESULT(vkCreatePipelineLayout(device(), &pipelineLayoutCreateInfo, nullptr, &m_layout));

	VkComputePipelineCreateInfo computePipelineInfo = {};
	computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineInfo.flags = 0;
	computePipelineInfo.basePipelineIndex = -1;
	computePipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	computePipelineInfo.layout = m_layout;
	computePipelineInfo.stage = shaderStageInfo;

	VK_CHECK_RESULT(vkCreateComputePipelines(device(), VK_NULL_HANDLE, 1, &computePipelineInfo, nullptr, &m_pipeline));
}

void ProceduralCompute::destroy(const vk::Device &device)
{
	vkDestroyPipeline(device(), m_pipeline, nullptr);
	vkDestroyPipelineLayout(device(), m_layout, nullptr);
	vkDestroyDescriptorPool(device(), m_descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device(), m_descriptorSetLayout, nullptr);
}

void ProceduralCompute::execute(const vk::CommandBuffer &cmdBuff, const vk::Context &context)
{
	{
		using namespace std::chrono;
		static time_point<steady_clock> time = steady_clock::now();
		m_pushc.width = context.getWidth();
		m_pushc.height = context.getHeight();
		m_pushc.time = duration_cast<milliseconds>(steady_clock::now() - time).count() / 1000.f;
	}
	vkCmdPushConstants(cmdBuff(), m_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstant), &m_pushc);
	vkCmdBindPipeline(cmdBuff(), VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
	vkCmdBindDescriptorSets(cmdBuff(), VK_PIPELINE_BIND_POINT_COMPUTE, m_layout, 0, 1, &m_descriptorSet, 0, 0);

	vkCmdDispatch(cmdBuff(), context.getWidth() / 16, context.getHeight() / 16, 1);
}

void ProceduralCompute::update(const vk::Device &device)
{
	VkDescriptorImageInfo descriptorInputImageInfo{};
	descriptorInputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	descriptorInputImageInfo.imageView = m_imageView;
	descriptorInputImageInfo.sampler = nullptr;


	std::vector<VkWriteDescriptorSet> descriptorWrites(m_descriptorBindings.size());
	for (size_t iBinding = 0; iBinding < m_descriptorBindings.size(); iBinding++)
	{
		descriptorWrites[iBinding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[iBinding].dstSet = m_descriptorSet;
		descriptorWrites[iBinding].dstBinding = m_descriptorBindings[iBinding].binding;
		descriptorWrites[iBinding].dstArrayElement = 0;
		descriptorWrites[iBinding].descriptorType = m_descriptorBindings[iBinding].descriptorType;
		descriptorWrites[iBinding].descriptorCount = m_descriptorBindings[iBinding].descriptorCount;
	}
	descriptorWrites[0].pImageInfo = &descriptorInputImageInfo;
	vkUpdateDescriptorSets(device(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

}