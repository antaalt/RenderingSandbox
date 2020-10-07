#pragma once

#include "../Framework/VulkanApi.h"
#include "../Framework/Window.h"
#include "../Framework/BaseApp.h"
#include "NodeGraph.h"

namespace node {

class Application : public engine::BaseApp
{
public:
	Application();
	~Application();

	void initialize() override;
	void destroy() override;

	void loop(vk::SwapChainFrame &frame) override;
private: // GUI
	void createRenderPass(const vk::Context &context);

	void drawNodeGUI();
private:
	void recreate();
	bool buildShaders();
	void createStages();
	void destroyStages();
private:
	std::vector<vk::CommandBuffer> m_commandBuffers;
	// GUI
	bool m_pause;
	VkRenderPass m_renderPass;
	std::vector<VkFramebuffer> m_frames;
	VkDescriptorPool m_descriptorPool;
	NodeGraph m_graph;
};

}