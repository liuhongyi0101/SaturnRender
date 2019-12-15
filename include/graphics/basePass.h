#pragma once
#include "vulkan/vulkan.h"
#include "VulkanInitializers.hpp"
#include "VulkanDevice.hpp"
#include "VulkanTools.h"
#include "VulkanTexture.hpp"
#include "VulkanModel.hpp"

#include <vector>
class BasePass
{
public:
	BasePass(vks::VulkanDevice * vulkanDevice);
	BasePass();
	~BasePass();

	vks::VulkanDevice * vulkanDevice;
	VkDevice device;
	VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;

	VkSemaphore semaphore = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipeline   pipeline;
	VkDescriptorSet descriptorSet;
	
	struct FrameBufferAttachment {
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
		VkFormat format;
		void destroy(VkDevice device)
		{
			vkDestroyImage(device, image, nullptr);
			vkDestroyImageView(device, view, nullptr);
			vkFreeMemory(device, mem, nullptr);
		}
	};
	struct FrameBuffer {
		int32_t width, height;
		VkFramebuffer frameBuffer;
		void setSize(int32_t w, int32_t h)
		{
			this->width = w;
			this->height = h;
		}
		void destroy(VkDevice device)
		{
			vkDestroyFramebuffer(device, frameBuffer, nullptr);
			 
		}
	};
	VkSampler colorSampler;
	VkPipelineLayout pipelineLayout;
	std::vector<VkShaderModule> shaderModules;
	void createAttachment(
		VkFormat format,
		VkImageUsageFlagBits usage,
		FrameBufferAttachment *attachment,
		uint32_t width,
		uint32_t height);
	VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin, VkCommandPool &cmdPool);
	void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free, VkCommandPool &cmdPool);

private :
	uint32_t testprivate;
protected:
	uint32_t testprotected;
};