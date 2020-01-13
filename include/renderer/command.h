#pragma once
#include "vulkan/vulkan.h"
#include "VulkanDevice.hpp"
#include "VulkanTools.h"
#include "VulkanInitializers.hpp"
#include "object/material.h"
#include "utils/loadasset.h"
#include "renderer/pipeline.h"
struct renderpassUnit {
	uint32_t width, height;
	std::shared_ptr<Pipeline> pipelineFact;
	VkRenderPass renderPass;
	MaterialPbr mat;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSet descriptorSet;
	VkDescriptorSet descriptorSetSkybox;
	vks::Model models;
	VkFramebuffer    frameBuffer;
	VkCommandBuffer commandBuffer;
};
class Command
{
public:
	Command(VkDevice &device);
	~Command();
	VkClearColorValue defaultClearColor = { { 0.025f, 0.025f, 0.025f, 1.0f } };

	void buildShadowMapCommandBuffer(renderpassUnit &renderPass);
	VkDevice device;
	
private:

};