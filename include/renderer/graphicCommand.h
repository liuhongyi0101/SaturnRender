#pragma once
#include "vulkan/vulkan.h"
#include "VulkanDevice.hpp"
#include "VulkanTools.h"
#include "VulkanInitializers.hpp"

class GraphicCommand
{
public:
	GraphicCommand(vks::VulkanDevice * vulkanDevice);
	~GraphicCommand();
	void createCommandPool();

	void addDrawCmd();
	void createCommandBuffer(VkCommandBufferLevel level);
	void startRecordCmd();
	void stopRecordCmd();
	VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
private:
	VkCommandPool cmdPool;
	vks::VulkanDevice * vulkanDevice;
	VkDevice device;
	
};
