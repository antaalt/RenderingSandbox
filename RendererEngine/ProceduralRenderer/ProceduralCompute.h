#pragma once

#include <array>

#include "VulkanApi.h"

#include "Geometry.h"

namespace app {

class ProceduralCompute
{
public:
	ProceduralCompute() : m_samples(0) {}

	void create(const vk::Context &context);
	void destroy(const vk::Context &context);

	void execute(const vk::ImageIndex &imageIndex, const vk::CommandBuffer &cmdBuff, const vk::Context &context);

	void update(const vk::ImageIndex &imageIndex, const vk::Context &context, const geo::mat4 &view);

	void reset();

	uint32_t getSampleCount() const { return m_samples; }

private:
	struct alignas(16) PushConstant
	{
		uint32_t samples;
		uint32_t width;
		uint32_t height;
		float time;
	} m_pushc;

	struct UniformBufferObject
	{
		geo::mat4 view;
		geo::mat4 proj;
		geo::mat4 viewInverse;
		geo::mat4 projInverse;
		geo::mat4 model;
	};

	uint32_t m_samples;

	VkPipeline m_pipeline;
	VkPipelineLayout m_layout;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorPool m_descriptorPool;
	VkDescriptorSet m_descriptorSet;
	std::vector<VkDescriptorSetLayoutBinding> m_descriptorBindings;

	std::vector<VkBuffer> m_uniformBuffers;
	std::vector<VkDeviceMemory> m_uniformBuffersMemory;
};

}