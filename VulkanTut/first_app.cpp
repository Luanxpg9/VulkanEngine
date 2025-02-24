#include "first_app.hpp"

// std
#include <stdexcept>
#include <array>
#include <iostream>

namespace lve {

	FirstApp::FirstApp() {
		loadModels();
		createPipelineLayout();
		createPipeline();
		createCommandBuffers();
	}

	FirstApp::~FirstApp() {
		vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
	}

	void FirstApp::run() {
		while (!lveWindow.shouldClose()) {
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(lveDevice.device());
	}

	void FirstApp::loadModels() {
		// Exercise 1: Create a function that draw a inverse triangle inside the triangle, the functions receives the number o decompositions;

		std::vector<LveModel::Vertex> vertices{
			{{0.0f, -0.5f}},
			{{0.5f, 0.5f}},
			{{-0.5f, 0.5f}}
		};

		breakTriangle(vertices, 4);

		lveModel = std::make_unique<LveModel>(lveDevice, vertices);
	}

	void FirstApp::createPipelineLayout() {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void FirstApp::createPipeline() {
		auto pipelineConfig = LvePipeline::defaultPipelineConfigInfo(lveSwapChain.width(), lveSwapChain.height());

		pipelineConfig.renderPass = lveSwapChain.getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		lvePipeline = std::make_unique<LvePipeline>(
			lveDevice,
			"simple_shader.vert.spv",
			"simple_shader.frag.spv",
			pipelineConfig);
	}

	void FirstApp::createCommandBuffers() {
		commandBuffers.resize(lveSwapChain.imageCount());
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = lveDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

		for (int i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = lveSwapChain.getRenderPass();
			renderPassInfo.framebuffer = lveSwapChain.getFrameBuffer(i);

			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = lveSwapChain.getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			lvePipeline->bind(commandBuffers[i]);
			lveModel->bind(commandBuffers[i]);
			lveModel->draw(commandBuffers[i]);

			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}

		}
	};

	void FirstApp::drawFrame() {
		uint32_t imageIndex;
		auto result = lveSwapChain.acquireNextImage(&imageIndex);
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		result = lveSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
	};

	std::vector<LveModel::Vertex> FirstApp::breakTriangle(std::vector<LveModel::Vertex>& vertices, int numberOfBreaks) {
		int vecSize = vertices.size();

		if (vecSize < 3) {
			throw std::runtime_error("invalid input");
		}

		std::vector<LveModel::Vertex> subVertices{};
		std::vector<LveModel::Vertex> newVertices{};

		for (int i = 0; i < numberOfBreaks; i++) {
			for (int j = 0; j < vertices.size(); j += 3) {
				glm::vec2 v1 = vertices[j].position;
				glm::vec2 v2 = vertices[j + 1].position;
				glm::vec2 v3 = vertices[j + 2].position;
				glm::vec2 leftPoint = { (v1.x + v2.x) / 2, (v1.y + v2.y) / 2 };
				glm::vec2 rightPoint = { (v1.x + v3.x) / 2, (v1.y + v3.y) / 2 };
				glm::vec2 bottomPoint = { (v2.x + v3.x) / 2, (v2.y + v3.y) / 2 };

				subVertices = {
					// T1
					{v1},
					{leftPoint},
					{rightPoint},
					// T2
					{leftPoint},
					{v2},
					{bottomPoint},
					// T3
					{rightPoint},
					{bottomPoint},
					{v3},
				};

				newVertices.insert(end(newVertices), begin(subVertices), end(subVertices));
			}

			vertices.clear();
			vertices.insert(end(vertices), begin(newVertices), end(newVertices));
			newVertices.clear();

		}
		
		return vertices;
	}
}