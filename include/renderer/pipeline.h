#pragma once
#include "vulkan/vulkan.h"
#include "VulkanInitializers.hpp"
#include "renderer/vertexDescriptions.h"
#include<array>
#include <map>
class Pipeline
{
public:
	Pipeline(VkDevice device);
	~Pipeline();
	std::map<std::string, VkPipeline> pipelines;
	void createPipelineCache(VkDevice device);
	void setupPipeline(VkDevice device,  std::shared_ptr<VertexDescriptions> vdo_);
	void createPipeline(VkDevice device, std::string &vertshader, std::string &fragshader, std::array<int, 2> &depthstate, VkPipeline &pipeline);
	void createSsaoPipeline(VkDevice device, VkRenderPass renderPass, VkPipelineLayout pipelineLayout);
	void createShadowPipeline(VkDevice device, VkRenderPass renderPass,VkPipelineLayout pipelineLayout, std::shared_ptr<VertexDescriptions> vdo);
	void createGbufferPipeline(VkDevice device, VkRenderPass renderPass, VkPipelineLayout pipelineLayout);
	void createSsaoBlurPipeline(VkDevice device, VkRenderPass renderPass, VkPipelineLayout pipelineLayout);
	void createQuadPipeline(VkDevice device, VkRenderPass renderPass, VkPipelineLayout pipelineLayout, std::string pipelinename);
	
#pragma region Field
	std::vector<VkShaderModule> shaderModules;
	VkRenderPass renderPass;

	VkPipelineLayout pipelineLayout;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
		vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

	VkPipelineRasterizationStateCreateInfo rasterizationState =
		vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);

	VkPipelineColorBlendAttachmentState blendAttachmentState =
		vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	    
	VkPipelineColorBlendStateCreateInfo colorBlendState =
		vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);

	VkPipelineDepthStencilStateCreateInfo depthStencilState =
		vks::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

	VkPipelineViewportStateCreateInfo viewportState =
		vks::initializers::pipelineViewportStateCreateInfo(1, 1);

	VkPipelineMultisampleStateCreateInfo multisampleState =
		vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);

	std::vector<VkDynamicState> dynamicStateEnables = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState =
		vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);


	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	
	VkGraphicsPipelineCreateInfo pipelineCreateInfo;
	std::array<int, 2 > depthstate = { 0, 0 };

	VkPipelineCache pipelineCache;
	VkPipelineVertexInputStateCreateInfo emptyInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
#pragma endregion
private:

};