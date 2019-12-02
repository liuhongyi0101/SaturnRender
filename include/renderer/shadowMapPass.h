#pragma once
#include<array>
#include "vulkan/vulkan.h"
#include "VulkanDevice.hpp"
#include "VulkanTools.h"
#include "VulkanInitializers.hpp"
// 16 bits of depth is enough for such a small scene
#define DEPTH_FORMAT VK_FORMAT_D32_SFLOAT
#define FB_COLOR_FORMAT VK_FORMAT_R8G8B8A8_UNORM
// Shadowmap properties
#define SHADOWMAP_DIM 2048

#define SHADOWMAP_FILTER VK_FILTER_LINEAR
class ShadowMapPass
{
public:
	ShadowMapPass(vks::VulkanDevice *vulkanDevice, VkCommandPool &cmdPool);
	~ShadowMapPass();
	void prepareShadowMapPass();
	void prepareShadowMapPassFramebuffer();
	VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin, VkCommandPool &cmdPool);
	VkRenderPass renderPass;
	VkDevice      device;
	vks::VulkanDevice *vulkanDevice;

	struct FrameBufferAttachment {
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
	};
	int32_t width, height;
	VkFramebuffer frameBuffer;
	FrameBufferAttachment depth;

	VkSampler depthSampler;
	VkDescriptorImageInfo descriptor;
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	// Semaphore used to synchronize between offscreen and final scene render pass
	VkSemaphore semaphore = VK_NULL_HANDLE;
private:

};
