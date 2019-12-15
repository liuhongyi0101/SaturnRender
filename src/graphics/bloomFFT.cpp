#include "graphics/bloomFFT.h"
#include "utils/loadshader.h"
BloomFFT::BloomFFT(vks::VulkanDevice * vulkanDevice)
{
	this->vulkanDevice = vulkanDevice;
	this->device = vulkanDevice->logicalDevice;
}

BloomFFT::~BloomFFT()
{
}

// Find and create a compute capable device queue

void BloomFFT::getComputeQueue()
{
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(vulkanDevice->physicalDevice, &queueFamilyCount, NULL);
	assert(queueFamilyCount >= 1);

	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	queueFamilyProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(vulkanDevice->physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

	// Some devices have dedicated compute queues, so we first try to find a queue that supports compute and not graphics
	bool computeQueueFound = false;
	for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
	{
		if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
		{
			compute.queueFamilyIndex = i;
			computeQueueFound = true;
			break;
		}
	}
	// If there is no dedicated compute queue, just find the first queue family that supports compute
	if (!computeQueueFound)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
		{
			if (queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				compute.queueFamilyIndex = i;
				computeQueueFound = true;
				break;
			}
		}
	}

	// Compute is mandatory in Vulkan, so there must be at least one queue family that supports compute
	assert(computeQueueFound);
	// Get a compute queue from the device
	vkGetDeviceQueue(device, compute.queueFamilyIndex, 0, &compute.queue);
}
// Prepare a texture target that is used to store compute shader calculations
void BloomFFT::prepareTextureTarget( uint32_t width, uint32_t height, VkCommandPool &cmdPool,VkQueue queue)
{
	

	// Prepare blit target texture
	textureComputeTarget.width = width;
	textureComputeTarget.height = height;

	VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageCreateInfo.extent = { width, height, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	// Image will be sampled in the fragment shader and used as storage target in the compute shader
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	imageCreateInfo.flags = 0;
	// Sharing mode exclusive means that ownership of the image does not need to be explicitly transferred between the compute and graphics queue
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
	VkMemoryRequirements memReqs;

	VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &textureComputeTarget.image));

	vkGetImageMemoryRequirements(device, textureComputeTarget.image, &memReqs);
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &textureComputeTarget.deviceMemory));
	VK_CHECK_RESULT(vkBindImageMemory(device, textureComputeTarget.image, textureComputeTarget.deviceMemory, 0));
	
	VkCommandBuffer layoutCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true, cmdPool);

	textureComputeTarget.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	vks::tools::setImageLayout(
		layoutCmd, textureComputeTarget.image,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		textureComputeTarget.imageLayout);

	flushCommandBuffer(layoutCmd, queue, true, cmdPool);

	// Create sampler
	VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 1.0f;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;
	sampler.maxLod =static_cast<float>(textureComputeTarget.mipLevels);
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr, &textureComputeTarget.sampler));

	// Create image view
	VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = VK_FORMAT_R8G8B8A8_UNORM;
	view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	view.image = textureComputeTarget.image;
	VK_CHECK_RESULT(vkCreateImageView(device, &view, nullptr, &textureComputeTarget.view));

	// Initialize a descriptor for later use
	textureComputeTarget.descriptor.imageLayout = textureComputeTarget.imageLayout;
	textureComputeTarget.descriptor.imageView = textureComputeTarget.view;
	textureComputeTarget.descriptor.sampler = textureComputeTarget.sampler;
	textureComputeTarget.device = vulkanDevice;
}

void BloomFFT::prepareCompute(VkDescriptorPool &descriptorPool, VkDescriptorImageInfo  &texDescriptor)
{
	getComputeQueue();

	// Create compute pipeline
	// Compute pipelines are created separate from graphics pipelines even if they use the same queue

	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		// Binding 0: Input image (read-only)
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0),
		// Binding 1: Output image (write)
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1),
	};

	VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &compute.descriptorSetLayout));

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
		vks::initializers::pipelineLayoutCreateInfo(&compute.descriptorSetLayout, 1);

	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &compute.pipelineLayout));

	VkDescriptorSetAllocateInfo allocInfo =
		vks::initializers::descriptorSetAllocateInfo(descriptorPool, &compute.descriptorSetLayout, 1);

	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &compute.descriptorSet));
	std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
		vks::initializers::writeDescriptorSet(compute.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &texDescriptor),
		vks::initializers::writeDescriptorSet(compute.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &textureComputeTarget.descriptor)
	};
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(computeWriteDescriptorSets.size()), computeWriteDescriptorSets.data(), 0, NULL);

	// Create compute shader pipelines
	VkComputePipelineCreateInfo computePipelineCreateInfo =
		vks::initializers::computePipelineCreateInfo(compute.pipelineLayout, 0);

	// One pipeline for each effect
	std::vector<std::string> shaderNames = { "emboss", "edgedetect", "sharpen" };
	for (auto& shaderName : shaderNames) {
		std::string fileName = getAssetPath + "computeshader/" + shaderName + ".comp.spv";
		computePipelineCreateInfo.stage = loadShader(fileName, VK_SHADER_STAGE_COMPUTE_BIT, device, shaderModules);
		VkPipeline pipeline;
		VK_CHECK_RESULT(vkCreateComputePipelines(device, 0, 1, &computePipelineCreateInfo, nullptr, &pipeline));
		compute.pipelines.push_back(pipeline);
	}

	// Separate command pool as queue family for compute may be different than graphics
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = compute.queueFamilyIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &compute.commandPool));

	// Create a command buffer for compute operations
	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		vks::initializers::commandBufferAllocateInfo(
			compute.commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1);

	VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &compute.commandBuffer));

	// Fence for compute CB sync
	VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &compute.fence));

	// Build a single command buffer containing the compute dispatch commands
	//buildComputeCommandBuffer();
}
void BloomFFT::buildComputeCommandBuffer()
{
	// Flush the queue if we're rebuilding the command buffer after a pipeline change to ensure it's not currently in use
	vkQueueWaitIdle(compute.queue);

	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	VK_CHECK_RESULT(vkBeginCommandBuffer(compute.commandBuffer, &cmdBufInfo));

	vkCmdBindPipeline(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelines[compute.pipelineIndex]);
	vkCmdBindDescriptorSets(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelineLayout, 0, 1, &compute.descriptorSet, 0, 0);

	vkCmdDispatch(compute.commandBuffer, textureComputeTarget.width / 16, textureComputeTarget.height / 16, 1);

	vkEndCommandBuffer(compute.commandBuffer);
}