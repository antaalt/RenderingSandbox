#pragma once

#include <array>

#include "VulkanApi.h"
#include "Scene.h"

namespace app {

class ProceduralCompute
{
public:
	ProceduralCompute() : m_samples(0) {}

	void create(const vk::Context &context);
	void destroy(const vk::Context &context);

	void execute(const vk::ImageIndex &imageIndex, const vk::CommandBuffer &cmdBuff, const vk::Context &context);

	// Reset the stage, set descriptor set. This must wait for all frames to end.
	void reset(const vk::Context &context, const Scene &scene);

	// Update the stage for the given image.
	void update(const vk::ImageIndex &imageIndex, const vk::Context &context, const Scene &scene);

	uint32_t getSampleCount() const { return m_samples; }

	VkImage getImage() { return m_image; }

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
		alignas(16) geo::mat4f view;
		alignas(16) geo::mat4f proj;
		alignas(16) geo::mat4f viewInverse;
		alignas(16) geo::mat4f projInverse;
		alignas(16) geo::mat4f model;
		alignas(16) geo::vec3f sunDir;
		float zNear;
		float zFar;
		float dt;
	};

	uint32_t m_samples;

	VkPipeline m_pipeline;
	VkPipelineLayout m_layout;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorPool m_descriptorPool;
	std::vector<VkDescriptorSetLayoutBinding> m_descriptorBindings;

	std::vector<VkDescriptorSet> m_descriptorSet;

	std::vector<VkBuffer> m_uniformBuffers;
	std::vector<VkDeviceMemory> m_uniformBuffersMemory;

	VkImage m_image;
	VkImageView m_imageView;
	VkDeviceMemory m_imageMemory;
};

}