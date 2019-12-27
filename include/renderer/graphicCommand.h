#pragma on
#include "vulkan/vulkan.h"
#include "VulkanDevice.hpp"
#include "VulkanTools.h"
#include "VulkanInitializers.hpp"

class GraphicCommand
{
public:
	GraphicCommand();
	~GraphicCommand();
	void createCommandPool();
	void addDrawCmd();
	VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin, VkCommandPool &cmdPool);
private:
	VkCommandPool cmdPool;
	vks::VulkanDevice * vulkanDevice;
	VkDevice device;
	VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
};
