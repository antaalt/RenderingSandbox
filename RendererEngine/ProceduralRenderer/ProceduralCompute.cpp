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

VkShaderModule createShaderModule(const VkDevice device, const std::string &filePath) {
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
	VK_CHECK_RESULT(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));
	return shaderModule;
}

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void ProceduralCompute::create(const vk::Context & context)
{
	// Descriptor set layout
	m_descriptorBindings.resize(2);

	m_descriptorBindings[0].binding = 0;
	m_descriptorBindings[0].descriptorCount = 1;
	m_descriptorBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	m_descriptorBindings[0].pImmutableSamplers = nullptr;
	m_descriptorBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	m_descriptorBindings[1].binding = 1;
	m_descriptorBindings[1].descriptorCount = 1;
	m_descriptorBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	m_descriptorBindings[1].pImmutableSamplers = nullptr;
	m_descriptorBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(m_descriptorBindings.size());
	layoutInfo.pBindings = m_descriptorBindings.data();

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(context.getLogicalDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout));

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

	VK_CHECK_RESULT(vkCreateDescriptorPool(context.getLogicalDevice(), &poolInfo, nullptr, &m_descriptorPool));

	// Descriptor set
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &m_descriptorSetLayout;

	VK_CHECK_RESULT(vkAllocateDescriptorSets(context.getLogicalDevice(), &allocInfo, &m_descriptorSet));

	// Pipeline
	VkShaderModule shaderModule = createShaderModule(context.getLogicalDevice(), "data/shaders/procedural.comp");

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

	VK_CHECK_RESULT(vkCreatePipelineLayout(context.getLogicalDevice(), &pipelineLayoutCreateInfo, nullptr, &m_layout));

	VkComputePipelineCreateInfo computePipelineInfo = {};
	computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineInfo.flags = 0;
	computePipelineInfo.basePipelineIndex = -1;
	computePipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	computePipelineInfo.layout = m_layout;
	computePipelineInfo.stage = shaderStageInfo;

	VK_CHECK_RESULT(vkCreateComputePipelines(context.getLogicalDevice(), VK_NULL_HANDLE, 1, &computePipelineInfo, nullptr, &m_pipeline));

	//uniformBuffer
	uint32_t imageCount = context.getImageCount();
	m_uniformBuffers.resize(imageCount);
	m_uniformBuffersMemory.resize(imageCount);

	for (uint32_t i = 0; i < imageCount; i++) {

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(UniformBufferObject);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK_RESULT(vkCreateBuffer(context.getLogicalDevice(), &bufferInfo, nullptr, &m_uniformBuffers[i]));

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(context.getLogicalDevice(), m_uniformBuffers[i], &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(context.getPhysicalDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VK_CHECK_RESULT(vkAllocateMemory(context.getLogicalDevice(), &allocInfo, nullptr, &m_uniformBuffersMemory[i]));

		VK_CHECK_RESULT(vkBindBufferMemory(context.getLogicalDevice(), m_uniformBuffers[i], m_uniformBuffersMemory[i], 0));

	}
}

void ProceduralCompute::destroy(const vk::Context &context)
{
	for (size_t i = 0; i < m_uniformBuffers.size(); i++)
	{
		vkDestroyBuffer(context.getLogicalDevice(), m_uniformBuffers[i], nullptr);
		vkFreeMemory(context.getLogicalDevice(), m_uniformBuffersMemory[i], nullptr);
	}
	vkDestroyPipeline(context.getLogicalDevice(), m_pipeline, nullptr);
	vkDestroyPipelineLayout(context.getLogicalDevice(), m_layout, nullptr);
	vkDestroyDescriptorPool(context.getLogicalDevice(), m_descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(context.getLogicalDevice(), m_descriptorSetLayout, nullptr);
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

void ProceduralCompute::update(const vk::Context &context, const geo::mat4 &viewInverse)
{
	// --- Descriptor set
	// Output
	VkDescriptorImageInfo descriptorInputImageInfo{};
	descriptorInputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	descriptorInputImageInfo.imageView = m_imageView;
	descriptorInputImageInfo.sampler = nullptr;
	// Ubo
	VkDescriptorBufferInfo descriptorCameraInfo{};
	descriptorCameraInfo.buffer = m_uniformBuffers[0];
	descriptorCameraInfo.range = sizeof(UniformBufferObject);

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
	descriptorWrites[1].pBufferInfo = &descriptorCameraInfo;
	vkUpdateDescriptorSets(context.getLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	// --- UBO
	float ratio = context.getWidth() / (float)context.getHeight();
	for (size_t iUbo = 0; iUbo < m_uniformBuffers.size(); iUbo++)
	{
		static UniformBufferObject ubo = {};
		ubo.view = geo::mat4::inverse(viewInverse);
		ubo.proj = geo::mat4::perspective(90.f, ratio, 0.1f, 1000.f);
		ubo.viewInverse = viewInverse;
		ubo.projInverse = geo::mat4::inverse(ubo.proj);
		ubo.model = geo::mat4::identity();
		
		void* data;
		VK_CHECK_RESULT(vkMapMemory(context.getLogicalDevice(), m_uniformBuffersMemory[iUbo], 0, sizeof(UniformBufferObject), 0, &data));
		memcpy(data, &ubo, sizeof(UniformBufferObject));
		vkUnmapMemory(context.getLogicalDevice(), m_uniformBuffersMemory[iUbo]);
	}
}

}