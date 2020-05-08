#include "Application.h"

#include <iostream>
#include <fstream>

#include "Geometry.h"

#include <imgui.h>
#include <examples\imgui_impl_glfw.h>
#include <examples\imgui_impl_vulkan.h>

namespace app {

std::vector<std::string> shaders = {
	"procedural.comp",
};

std::vector<char> loadFile(const std::string &str)
{
	std::ifstream file(str, std::ios::binary | std::ios::ate);
	std::streampos size = file.tellg();
	file.seekg(0);
	std::vector<char> content(size);
	file.read(content.data(), content.size());
	return content;
}

Application::Application() :
	m_window(),
	m_context(m_window),
	m_compute(),
	m_transform(geo::mat4::translate(geo::vec3(0, 5, 0)))
{

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
	m_gui.create(m_context, m_window);


	if (!buildShaders())
		throw std::runtime_error("Cannot build shaders");
	createStages();
}


Application::~Application()
{
	destroyStages();
	m_gui.destroy(m_context);
	std::vector<VkCommandBuffer> commandBuffers(m_context.getImageCount());
	for (uint32_t iImage = 0; iImage < m_context.getImageCount(); iImage++)
		commandBuffers[iImage] = m_commandBuffers[iImage]();

	vkFreeCommandBuffers(m_context.getLogicalDevice(), m_context.getCommandPool(), m_context.getImageCount(), commandBuffers.data());
}

void submit(VkDevice device, VkQueue queue, const vk::SwapChainFrame &frame, const vk::CommandBuffer &commandBuffer) {
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

	VK_CHECK_RESULT(vkResetFences(device, 1, &frame.inFlightFence));
	VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, frame.inFlightFence));
}

bool Application::inputs()
{
	// move to gui ?
	bool updated = false;

	const float sensitivity = 0.01f;
	const ImGuiIO &io = ImGui::GetIO();
	// ROTATE
	if (io.MouseDown[0] && !io.WantCaptureMouse)
	{
		m_transform = m_transform * geo::mat4::rotate(geo::vec3(0.f, 1.f, 0.f), math::Radian(sensitivity*io.MouseDelta.x));
		m_transform = m_transform * geo::mat4::rotate(geo::vec3(1.f, 0.f, 0.f), math::Radian(sensitivity*io.MouseDelta.y));
		updated = true;
	}
	// PAN
	if (io.MouseDown[1] && !io.WantCaptureMouse)
	{
		m_transform = m_transform *geo::mat4::translate(geo::vec3(
			-io.MouseDelta.x,
			io.MouseDelta.y,
			0.f
		));
		updated = true;
	}
	// MOVE
	if (!io.WantCaptureKeyboard)
	{
		bool keyForward = io.KeysDown[GLFW_KEY_W];
		bool keyBackward = io.KeysDown[GLFW_KEY_S];
		bool keyLeft = io.KeysDown[GLFW_KEY_A];
		bool keyRight = io.KeysDown[GLFW_KEY_D];
		bool keyUp = io.KeysDown[GLFW_KEY_Q];
		bool keyDown = io.KeysDown[GLFW_KEY_E];
		m_transform = m_transform * geo::mat4::translate(geo::vec3(
			static_cast<float>(keyRight - keyLeft), // left right
			static_cast<float>(keyUp - keyDown), // up down
			static_cast<float>(keyForward - keyBackward) // forward backward
		) * 10.f);
		if (keyForward || keyBackward || keyLeft || keyRight || keyUp || keyDown)
			updated = true;
	}
	// ZOOM
	if (io.MouseWheel != 0.0 && !io.WantCaptureMouse)
	{
		m_transform = m_transform * geo::mat4::translate(geo::vec3(0.f, 0.f, io.MouseWheel * 100.f));
		updated = true;
	}
	if (io.KeysDown[GLFW_KEY_SPACE])
	{
		m_compute.reset();
		recreate();
		updated = true;
	}
	return updated;
}

void Application::execute()
{
	m_window.loop([&]() {

		m_gui.newFrame();
		if (inputs())
		{
			m_compute.reset();
		}
		Stats stats;
		stats.samples = m_compute.getSampleCount();
		m_gui.draw(stats);

		// Render
		vk::SwapChainFrame frame;
		m_context.acquireNextFrame(&frame);

		m_compute.update(frame.imageIndex, m_context, m_transform);

		vk::CommandBuffer &cmdBuff = m_commandBuffers[frame.imageIndex()];
		cmdBuff.begin();
		m_compute.execute(frame.imageIndex, cmdBuff, m_context);

		VkImageMemoryBarrier imageMemoryBarrier[2]{};
		// barrier render target
		imageMemoryBarrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier[0].srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		imageMemoryBarrier[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageMemoryBarrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier[0].image = m_compute.getImage();
		imageMemoryBarrier[0].subresourceRange = VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		// barrier swapchain
		imageMemoryBarrier[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier[1].srcAccessMask = 0;
		imageMemoryBarrier[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier[1].oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		imageMemoryBarrier[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier[1].image = m_context.getImage(frame.imageIndex);
		imageMemoryBarrier[1].subresourceRange = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		vkCmdPipelineBarrier(
			cmdBuff(),
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			2, imageMemoryBarrier
		);

		VkImageCopy copyRegion{};
		VkImageSubresourceLayers subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copyRegion.extent = VkExtent3D{m_context.getWidth(), m_context.getHeight(), 1 };
		copyRegion.srcSubresource = subResource;
		copyRegion.dstSubresource = subResource;
		vkCmdCopyImage(
			cmdBuff(),
			m_compute.getImage(),
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			m_context.getImage(frame.imageIndex),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &copyRegion
		);

		// barrier swapchain
		imageMemoryBarrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		imageMemoryBarrier[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier[0].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		imageMemoryBarrier[0].image = m_compute.getImage();
		imageMemoryBarrier[0].subresourceRange = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		vkCmdPipelineBarrier(
			cmdBuff(),
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, imageMemoryBarrier
		);
		
		cmdBuff.end();
		submit(m_context.getLogicalDevice(), m_context.getGraphicQueue(), frame, cmdBuff);

		m_gui.render(frame.imageIndex, m_context);

		m_context.presentFrame(frame);
	});
}

void Application::createStages()
{
	// Register
	for (const std::string &shader : shaders)
		m_context.registerShader(shader, loadFile("data/shaders/" + shader + ".spv"));
	// Pass
	m_compute.create(m_context);

	// Clean
	m_context.destroyShaders();
}

void Application::destroyStages()
{
	m_compute.destroy(m_context);
}

void Application::recreate()
{
	if (!buildShaders())
		return;
	destroyStages();
	createStages();
}

bool Application::buildShaders()
{
	std::cout << "--- Building shaders..." << std::endl;
	for (const std::string &shader : shaders)
	{
		std::string filePath = "data/shaders/" + shader;
		std::string buildPath = filePath + ".spv";
		char *env;
		size_t size;
		_dupenv_s(&env, &size, "VULKAN_SDK");
		if (size == 0)
			return false;
		char buffer[256];
		snprintf(buffer, 256, "%s/Bin32/glslangValidator.exe -V %s -o %s", env, filePath.c_str(), buildPath.c_str());
		if (0 != system(buffer))
			return false;
	}
	return true;
}


void GUI::create(const vk::Context &context, const app::Window &window)
{
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
		VK_CHECK_RESULT(vkCreateDescriptorPool(context.getLogicalDevice(), &pool_info, nullptr, &m_descriptorPool));
	}

	createRenderPass(context);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForVulkan(window.getHandle(), true);
	ImGui_ImplVulkan_InitInfo info{};
	info.Instance = context.getInstance();
	info.PhysicalDevice = context.getPhysicalDevice();
	info.Device = context.getLogicalDevice();
	info.QueueFamily = context.getGraphicQueueHandle();
	info.Queue = context.getGraphicQueue();
	info.PipelineCache = VK_NULL_HANDLE;
	info.DescriptorPool = m_descriptorPool;
	info.MinImageCount = 2; // >= 2
	info.ImageCount = static_cast<uint32_t>(context.getImageCount()); // >= MinImageCount
	info.CheckVkResultFn = [](VkResult err) {
		if (VK_SUCCESS != err) {
			throw std::runtime_error(vkGetErrorString(err).c_str());
		}
	};
	ImGui_ImplVulkan_Init(&info, m_renderPass);

	// Setup Style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	VkCommandBuffer cmdBuffer = context.createSingleTimeCommand();

	ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);

	context.endSingleTimeCommand(cmdBuffer);

	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void GUI::destroy(const vk::Context &context)
{
	ImGui_ImplVulkan_Shutdown();
	for (uint32_t i = 0; i < m_frames.size(); i++)
	{
		vkDestroyFramebuffer(context.getLogicalDevice(), m_frames[i], nullptr);
	}
	vkDestroyRenderPass(context.getLogicalDevice(), m_renderPass, nullptr);
	vkDestroyDescriptorPool(context.getLogicalDevice(), m_descriptorPool, nullptr);
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void GUI::newFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void GUI::draw(const Stats &stats)
{
	static bool open = true;
	if (ImGui::Begin("Stats", &open))
	{
		ImGuiIO &io = ImGui::GetIO();
		ImGui::Text("%.1f FPS", io.Framerate);
		ImGui::Text("Samples : %u", stats.samples);
	}
	ImGui::End();
}

void GUI::render(const vk::ImageIndex &imageIndex, vk::Context &context)
{
	// Rendering
	ImGui::Render();
	VkCommandBuffer cmdBuff = context.createSingleTimeCommand();
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_frames[imageIndex()];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = VkExtent2D{ context.getWidth(), context.getHeight() };
	vkCmdBeginRenderPass(cmdBuff, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuff);

	vkCmdEndRenderPass(cmdBuff);

	context.endSingleTimeCommand(cmdBuff);
}

void GUI::createRenderPass(const vk::Context &context)
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

}