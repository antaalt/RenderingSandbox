#pragma once

#include "Window.h"
#include "VulkanApi.h"
#include "ProceduralCompute.h"
#include "Geometry.h"
#include "Scene.h"

namespace app {

struct Stats {
	uint32_t samples;
};

struct GUI {
	GUI() : m_pause(false) {}
	void create(const vk::Context &context, const app::Window &window);
	void destroy(const vk::Context &context);

	void newFrame();
	bool draw(const Stats &stats);
	void render(const vk::ImageIndex &imageIndex, vk::Context &context);
	void setScene(Scene *scene) { m_scene = scene; }
	bool isPaused() const { return m_pause; }
private:
	void createRenderPass(const vk::Context &context);
private:
	Scene *m_scene;
private:
	bool m_pause;
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
	Scene m_scene;
	GUI m_gui;
};

}