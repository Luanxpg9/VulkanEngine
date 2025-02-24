#pragma once

#include "lve_window.hpp"
#include "lve_pipeline.hpp"
#include "lve_swap_chain.hpp"
#include "lve_device.hpp"
#include "lve_model.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		FirstApp();
		~FirstApp();

		FirstApp(const LveWindow&) = delete;
		FirstApp& operator=(const LveWindow&) = delete;
		void run();

	private:
		void loadModels();
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void drawFrame();
		std::vector<LveModel::Vertex> breakTriangle(std::vector<LveModel::Vertex>& vertices, int numberOfBreaks = 1);


		LveWindow lveWindow{ WIDTH, HEIGHT, "Hello Vulkan!" };
		LveDevice lveDevice{ lveWindow };
		LveSwapChain lveSwapChain{ lveDevice, lveWindow.getExtent() };
		std::unique_ptr<LvePipeline> lvePipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;
		std::unique_ptr<LveModel> lveModel;
	};
}