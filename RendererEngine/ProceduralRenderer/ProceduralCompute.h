#pragma once

#include <array>

#include "VulkanApi.h"

namespace app {

class ProceduralCompute
{
public:

	void create(const vk::Device &device);
	void destroy(const vk::Device &device);

	void execute(const vk::CommandBuffer &cmdBuff, const vk::Context &context);

	void update(const vk::Device &device);

	void setOutput(VkImageView imageView) { m_imageView = imageView; }

private:
	struct alignas(16) PushConstant
	{
		float what;
	} m_pushc;

	VkPipeline m_pipeline;
	VkPipelineLayout m_layout;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorPool m_descriptorPool;
	VkDescriptorSet m_descriptorSet;
	std::vector<VkDescriptorSetLayoutBinding> m_descriptorBindings;

	VkImageView m_imageView;
};

}