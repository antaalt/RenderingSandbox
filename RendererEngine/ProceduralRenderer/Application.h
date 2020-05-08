#pragma once

#include "Window.h"
#include "VulkanApi.h"
#include "ProceduralCompute.h"
#include "Geometry.h"

namespace app {
struct Stats {
	uint32_t samples;
};

struct GUI {
	void create(const vk::Context &context, const app::Window &window);
	void destroy(const vk::Context &context);

	void newFrame();
	void draw(const Stats &stats);
	void render(uint32_t imageIndex, vk::Context &context);
private:
	void createRenderPass(const vk::Context &context);
private:
	VkRenderPass m_renderPass;
	std::vector<VkFramebuffer> m_frames;
	VkDescriptorPool m_descriptorPool;
};

class Application
{
public:
	Application();
	~Application();

	bool inputs();
	void execute();

	void recreate();
	bool buildShaders();
	void createStages();
	void destroyStages();
private:
	Window m_window;
	vk::Context m_context;
	ProceduralCompute m_compute;
	std::vector<vk::CommandBuffer> m_commandBuffers;
	geo::mat4 m_transform;
	GUI m_gui;
};

}