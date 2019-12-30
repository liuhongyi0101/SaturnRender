#include "renderer/graphicCommand.h"

GraphicCommand::GraphicCommand(vks::VulkanDevice * vulkanDevice)
{
	this->vulkanDevice = vulkanDevice;
	this->device = vulkanDevice->logicalDevice;
	createCommandPool();
	createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}

GraphicCommand::~GraphicCommand()
{
}
void GraphicCommand::createCommandPool()
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = 0;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool));
}
void GraphicCommand::createCommandBuffer(VkCommandBufferLevel level)
{
	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		vks::initializers::commandBufferAllocateInfo(
			cmdPool,
			level,
			1);
	VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &cmdBuffer));
	startRecordCmd();
}
void GraphicCommand::startRecordCmd()
{

	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
}
void GraphicCommand::stopRecordCmd()
{

	VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));

}
void GraphicCommand::addDrawCmd()
{





}