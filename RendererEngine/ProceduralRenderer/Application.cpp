#include "Application.h"

#include <iostream>

namespace app {

Application::Application() :
	m_window(),
	m_context(m_window),
	m_compute()
{
	m_compute.create(m_context.getDevice());

	const uint32_t imageCount = static_cast<uint32_t>(m_context.getImageCount());
	const uint32_t commandBufferCount = imageCount * 2;
	std::vector<VkCommandBuffer> commandBuffers(commandBufferCount);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_context.getDevice().getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = commandBufferCount;

	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_context.getDevice()(), &allocInfo, commandBuffers.data()));

	m_commandBuffers.resize(imageCount);
	for (uint32_t iImage = 0; iImage < imageCount; iImage++)
	{
		m_commandBuffers[iImage].set(commandBuffers[iImage], iImage);
	}
}


Application::~Application()
{
	m_compute.destroy(m_context.getDevice());
}

void submit(const vk::Device &device, const vk::Queue &queue, const vk::SwapChainFrame &frame, const vk::CommandBuffer &commandBuffer) {
	VkCommandBuffer cmdBuff = commandBuffer();
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuff;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &frame.renderFinishedSemaphore;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &frame.imageAvailableSemaphore;

	VK_CHECK_RESULT(vkResetFences(device(), 1, &frame.inFlightFence));
	VK_CHECK_RESULT(vkQueueSubmit(queue(), 1, &submitInfo, frame.inFlightFence));
}

void Application::execute()
{
	m_window.loop([&]() {
		static int i = 0;
		vk::SwapChainFrame frame;
		m_context.acquireNextFrame(&frame);

		m_compute.setOutput(m_context.getImageView(frame.imageIndex));
		m_compute.update(m_context.getDevice());

		vk::CommandBuffer cmdBuff = m_commandBuffers[frame.imageIndex];
		cmdBuff.begin();
		m_compute.execute(cmdBuff, m_context);
		cmdBuff.end();

		submit(m_context.getDevice(), m_context.getDevice().getGraphicQueue(), frame, cmdBuff);

		m_context.presentFrame(frame);

		std::cout << "LOOP RUNNING " << i++ << std::endl;
	});
}


}