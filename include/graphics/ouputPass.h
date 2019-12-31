#pragma once
#include "graphics/basePass.h"
#include "VulkanSwapChain.hpp"
class  OutputPass :public BasePass
{
public:
	 OutputPass(vks::VulkanDevice * vulkanDevice);


	 void createRenderPass(VkFormat depthFormat, VkFormat colorFormat);
	 void createFrameBuffer(VulkanSwapChain &swapChain,uint32_t width, uint32_t height,VkCommandPool cmdpool);
	 void createPipeline();
	 void createDescriptorsLayouts();
	 void wirteDescriptorSets(VkDescriptorPool &descriptorPool, VkDescriptorImageInfo &texDescriptor);
	 void createUniformBuffers(VkQueue queue);
	 void updateUniformBufferMatrices();
	 void buildCommandBuffer(VkCommandBuffer& cmdBuffer, uint32_t width, uint32_t height);
	~ OutputPass();

	std::vector<VkCommandBuffer> drawCmdBuffers;
	// Global render pass for frame buffer writes
	VkRenderPass renderPass;
	// List of available frame buffers (same as number of swap chain images)
	std::vector<VkFramebuffer>frameBuffers;

private:
	struct UBOParams {
		float count;
	} uboParams;
	vks::Buffer uniformBuffers;

};