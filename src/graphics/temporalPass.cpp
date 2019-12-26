#include "graphics/temporalPass.h"


TemporalPass::TemporalPass(vks::VulkanDevice * vulkanDevice)
{
	this->vulkanDevice = vulkanDevice;
	this->device = vulkanDevice->logicalDevice;
}

TemporalPass::~TemporalPass()
{
}