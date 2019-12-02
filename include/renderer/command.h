#pragma once
#include "vulkan/vulkan.h"
#include "VulkanDevice.hpp"
#include "VulkanTools.h"
#include "VulkanInitializers.hpp"
#include "bobject/material.h"
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
	Meshes models;
	VkFramebuffer    frameBuffer;
	VkCommandBuffer commandBuffer;
};
class Command
{
public:
	Command(VkDevice &device);
	~Command();
	VkClearColorValue defaultClearColor = { { 0.025f, 0.025f, 0.025f, 1.0f } };
	void buildCommandBuffers(renderpassUnit renderPass, std::vector<VkCommandBuffer>& drawCmdBuffers, std::vector<VkFramebuffer> &frameBuffers);
	void buildShadowMapCommandBuffer(renderpassUnit &renderPass);
	void ssaobuildCommandBuffers(renderpassUnit renderPass, std::vector<VkCommandBuffer>& drawCmdBuffers, std::vector<VkFramebuffer> &frameBuffers);
	VkDevice device;
	
private:

};