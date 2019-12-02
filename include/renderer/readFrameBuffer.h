#pragma once
#include <vulkan/vulkan.h>
#include "VulkanDevice.hpp"
#include "VulkanBuffer.hpp"
class readFrameBuffer
{
public:
	readFrameBuffer(vks::VulkanDevice *vulkanDevice, VkQueue queue);
	~readFrameBuffer();
	unsigned char* readPixel(VkImage image, uint32_t & width, uint32_t & height);
	void destoryDstImageMemory();
private:
	VkDevice device;
	VkQueue queue;
	vks::VulkanDevice *vulkanDevice;
	VkDeviceMemory dstImageMemory;
	VkImage dstImage;
	void createDstImage(uint32_t & width, uint32_t & height);
};
