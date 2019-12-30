#include"graphics/deferedShading.h"
#include"utils/loadshader.h"

DeferredShading::DeferredShading(vks::VulkanDevice * vulkanDevice)
{
	this->vulkanDevice = vulkanDevice;
	this->device = vulkanDevice->logicalDevice;
}

DeferredShading::~DeferredShading()
{
}
void DeferredShading::createRenderPass(uint32_t width, uint32_t  height)
{

	const uint32_t ssaoWidth = width;
	const uint32_t ssaoHeight = height;

	VkFormat depthFormat;

	// Find a suitable depth format
	VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(vulkanDevice->physicalDevice, &depthFormat);
	assert(validDepthFormat);
	deferredShadingRtFrameBuffer.setSize(width, height);

	createAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &deferredShadingRtFrameBuffer.deferredShadingRtAttachment, width, height);
	

	std::array<VkAttachmentDescription, 2> attachments = {};
	// Color attachment
	attachments[0].format = deferredShadingRtFrameBuffer.deferredShadingRtAttachment.format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	// Depth attachment
	attachments[1].format = depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;
	subpassDescription.pResolveAttachments = nullptr;

	// Subpass dependencies for layout transitions
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));


}
void DeferredShading::createFrameBuffer(VkImageView &depthView)
{
	VkImageView attachments[2];
	attachments[0] = deferredShadingRtFrameBuffer.deferredShadingRtAttachment.view;
	attachments[1] = depthView;

	VkFramebufferCreateInfo fbufCreateInfo = vks::initializers::framebufferCreateInfo();
	fbufCreateInfo.renderPass = renderPass;
	fbufCreateInfo.pAttachments = attachments;
	fbufCreateInfo.attachmentCount = 2;
	fbufCreateInfo.width = deferredShadingRtFrameBuffer.width;
	fbufCreateInfo.height = deferredShadingRtFrameBuffer.height;
	fbufCreateInfo.layers = 1;
	VK_CHECK_RESULT(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &deferredShadingRtFrameBuffer.frameBuffer));

	// Shared sampler used for all color attachments
	VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
	sampler.magFilter = VK_FILTER_NEAREST;
	sampler.minFilter = VK_FILTER_NEAREST;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 1.0f;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr, &colorSampler));

}
void DeferredShading::createDescriptorsLayouts()
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
	VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo;
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo();
	setLayoutBindings = {
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),						// FS Position+Depth
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),						// FS Normals
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),						// FS Albedo

			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),						// FS SSAO blurred
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4),						// FS IrradianceMap
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 5),						// FS prefilteredCube
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 6),
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 7),  // FS lutMap
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 8),
			
	};
	setLayoutCreateInfo = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), static_cast<uint32_t>(setLayoutBindings.size()));
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &setLayoutCreateInfo, nullptr, &descriptorSetLayout));

	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

}
void DeferredShading::createPipeline()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_ALWAYS);
	VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
	VkPipelineVertexInputStateCreateInfo emptyInputState = vks::initializers::pipelineVertexInputStateCreateInfo();

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(pipelineLayout, renderPass, 0);
	pipelineCreateInfo.pVertexInputState = &emptyInputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();

	std::array<VkPipelineColorBlendAttachmentState, 1> blendAttachmentStates = {
		vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
	};
	colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
	colorBlendState.pAttachments = blendAttachmentStates.data();

	shaderStages[0] = loadShader(getAssetPath + "common/fullscreen.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, device, shaderModules);
	shaderStages[1] = loadShader(getAssetPath + "deferredShading/deferredShading.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, device, shaderModules);

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, 0, 1, &pipelineCreateInfo, nullptr, &pipeline));

}
void DeferredShading::createUniformBuffers(VkQueue queue, glm::vec4 &lightPos)
{
	// Scene matrices
	vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformBuffers,
		sizeof(uboParams));

	updateUniformBufferMatrices(lightPos);

}
void DeferredShading::wirteDescriptorSets(VkDescriptorPool &descriptorPool, std::vector<VkDescriptorImageInfo> &texDescriptor, std::vector<VkDescriptorImageInfo> &iblTexDescriptor)
{
	VkDescriptorSetAllocateInfo descriptorAllocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, nullptr, 1);
	std::vector<VkWriteDescriptorSet> writeDescriptorSets;
	descriptorAllocInfo.pSetLayouts = &descriptorSetLayout;
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorAllocInfo, &descriptorSet));
	writeDescriptorSets = {
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &texDescriptor[0]),			
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &texDescriptor[1]),			
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &texDescriptor[2]),         
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &texDescriptor[3]),			
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, &texDescriptor[4]),			
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, &iblTexDescriptor[0]),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, &iblTexDescriptor[1]),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 7, &iblTexDescriptor[2]),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8, &uniformBuffers.descriptor),
	};
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
}
void DeferredShading::updateUniformBufferMatrices(glm::vec4 &lightPos)
{
	
	uboParams.lightPos = lightPos;
	VK_CHECK_RESULT(uniformBuffers.map());
	uniformBuffers.copyTo(&uboParams, sizeof(uboParams));
	uniformBuffers.unmap();
}

void DeferredShading::buildCommandBuffer(VkCommandBuffer &cmdBuffer)
{

	
	std::vector<VkClearValue> clearValues(1);
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = deferredShadingRtFrameBuffer.frameBuffer;
	renderPassBeginInfo.renderArea.extent.width = deferredShadingRtFrameBuffer.width;
	renderPassBeginInfo.renderArea.extent.height = deferredShadingRtFrameBuffer.height;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();


	vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = vks::initializers::viewport((float)deferredShadingRtFrameBuffer.width, (float)deferredShadingRtFrameBuffer.height, 0.0f, 1.0f);
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

	VkRect2D scissor = vks::initializers::rect2D(deferredShadingRtFrameBuffer.width, deferredShadingRtFrameBuffer.height, 0, 0);
	vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

	vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
	

}