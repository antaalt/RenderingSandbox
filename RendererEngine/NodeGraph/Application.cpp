#include "Application.h"

#include <imgui.h>
#include <examples\imgui_impl_glfw.h>
#include <examples\imgui_impl_vulkan.h>
#include <imnodes.h>

namespace node {


Application::Application() :
	BaseApp()
{
	initialize();
}


Application::~Application()
{
	destroy();
}

void Application::createRenderPass(const vk::Context &context)
{
	for (uint32_t i = 0; i < m_frames.size(); i++)
		vkDestroyFramebuffer(context.getLogicalDevice(), m_frames[i], nullptr);

	if (m_renderPass != VK_NULL_HANDLE)
		vkDestroyRenderPass(context.getLogicalDevice(), m_renderPass, nullptr);

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = context.getFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorReference;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	VK_CHECK_RESULT(vkCreateRenderPass(context.getLogicalDevice(), &renderPassInfo, nullptr, &m_renderPass))


		uint32_t imageCount = context.getImageCount();
	m_frames.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; i++)
	{
		std::vector<VkImageView> att(1);
		att[0] = context.getImageView(vk::ImageIndex(i));

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(att.size());
		framebufferInfo.pAttachments = att.data();
		framebufferInfo.width = context.getWidth();
		framebufferInfo.height = context.getHeight();
		framebufferInfo.layers = 1;

		VK_CHECK_RESULT(vkCreateFramebuffer(context.getLogicalDevice(), &framebufferInfo, nullptr, &m_frames[i]));
	}
}

void Application::drawNodeGUI()
{
	m_graph.draw();
}

void Application::initialize()
{
	// Create command buffers
	const uint32_t imageCount = static_cast<uint32_t>(m_context.getImageCount());
	const uint32_t commandBufferCount = imageCount * 2;
	std::vector<VkCommandBuffer> commandBuffers(commandBufferCount);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_context.getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = commandBufferCount;

	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_context.getLogicalDevice(), &allocInfo, commandBuffers.data()));

	m_commandBuffers.resize(imageCount);
	for (uint32_t iImage = 0; iImage < imageCount; iImage++)
	{
		m_commandBuffers[iImage].set(commandBuffers[iImage], vk::ImageIndex(iImage));
	}
	// Create gui
	{ // Custom descriptor pool for imgui
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		VK_CHECK_RESULT(vkCreateDescriptorPool(m_context.getLogicalDevice(), &pool_info, nullptr, &m_descriptorPool));
	}

	createRenderPass(m_context);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
														   //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

														   // Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForVulkan(m_window.getHandle(), true);
	ImGui_ImplVulkan_InitInfo info{};
	info.Instance = m_context.getInstance();
	info.PhysicalDevice = m_context.getPhysicalDevice();
	info.Device = m_context.getLogicalDevice();
	info.QueueFamily = m_context.getGraphicQueueHandle();
	info.Queue = m_context.getGraphicQueue();
	info.PipelineCache = VK_NULL_HANDLE;
	info.DescriptorPool = m_descriptorPool;
	info.MinImageCount = 2; // >= 2
	info.ImageCount = static_cast<uint32_t>(m_context.getImageCount()); // >= MinImageCount
	info.CheckVkResultFn = [](VkResult err) {
		if (VK_SUCCESS != err) {
			throw std::runtime_error(vkGetErrorString(err).c_str());
		}
	};
	ImGui_ImplVulkan_Init(&info, m_renderPass);

	// Setup Style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	VkCommandBuffer cmdBuffer = m_context.createSingleTimeCommand();

	ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);

	m_context.endSingleTimeCommand(cmdBuffer);

	ImGui_ImplVulkan_DestroyFontUploadObjects();

	imnodes::Initialize();
}

void Application::destroy()
{
	std::vector<VkCommandBuffer> commandBuffers(m_context.getImageCount());
	for (uint32_t iImage = 0; iImage < m_context.getImageCount(); iImage++)
		commandBuffers[iImage] = m_commandBuffers[iImage]();

	vkFreeCommandBuffers(m_context.getLogicalDevice(), m_context.getCommandPool(), m_context.getImageCount(), commandBuffers.data());
	// GUI
	imnodes::Shutdown();
	ImGui_ImplVulkan_Shutdown();
	for (uint32_t i = 0; i < m_frames.size(); i++)
	{
		vkDestroyFramebuffer(m_context.getLogicalDevice(), m_frames[i], nullptr);
	}
	vkDestroyRenderPass(m_context.getLogicalDevice(), m_renderPass, nullptr);
	vkDestroyDescriptorPool(m_context.getLogicalDevice(), m_descriptorPool, nullptr);
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Application::loop(vk::SwapChainFrame &frame)
{
	// New frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	vk::CommandBuffer &cmdBuff = m_commandBuffers[frame.imageIndex()];
	cmdBuff.begin();

	// Draw
	{
		VkClearColorValue clearColor{ 0.f, 0.f, 0.f, 1.f };
		VkImageSubresourceRange subResource = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		
		VkImageMemoryBarrier memoryBarrier{};
		memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		memoryBarrier.srcAccessMask = 0;
		memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		memoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		memoryBarrier.image = m_context.getImage(frame.imageIndex);
		memoryBarrier.subresourceRange = subResource;
		vkCmdPipelineBarrier(
			cmdBuff(),
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &memoryBarrier
		);

		vkCmdClearColorImage(cmdBuff(), m_context.getImage(frame.imageIndex), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &subResource);

		memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		memoryBarrier.dstAccessMask = 0;
		memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		memoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		memoryBarrier.image = m_context.getImage(frame.imageIndex);
		memoryBarrier.subresourceRange = subResource;
		vkCmdPipelineBarrier(
			cmdBuff(),
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &memoryBarrier
		);

		drawNodeGUI();
	}
	// Rendering
	ImGui::Render();
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_frames[frame.imageIndex()];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = VkExtent2D{ m_context.getWidth(), m_context.getHeight() };
	vkCmdBeginRenderPass(cmdBuff(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuff());

	vkCmdEndRenderPass(cmdBuff());

	cmdBuff.end();

	// submit
	VkCommandBuffer cmd = cmdBuff();
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &frame.renderFinishedSemaphore;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &frame.imageAvailableSemaphore;

	VK_CHECK_RESULT(vkQueueSubmit(m_context.getGraphicQueue(), 1, &submitInfo, VK_NULL_HANDLE));
}

}