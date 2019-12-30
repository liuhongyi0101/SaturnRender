#include "graphics/gbufferPass.h"
#include <array>
#include "utils/loadshader.h"
GbufferPass::GbufferPass(vks::VulkanDevice * vulkanDevice)
{
	this->vulkanDevice = vulkanDevice;
	this->device = vulkanDevice->logicalDevice;
}

GbufferPass::~GbufferPass()
{
}
void GbufferPass::createFramebuffersAndRenderPass(uint32_t width, uint32_t  height)
{

	const uint32_t ssaoWidth = width;
	const uint32_t ssaoHeight = height;

	deferredFrameBuffers.setSize(width, height);
	// Find a suitable depth format
	VkFormat attDepthFormat;
	VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(vulkanDevice->physicalDevice, &attDepthFormat);
	assert(validDepthFormat);

	// G-Buffer 
	createAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &deferredFrameBuffers.position, width, height);	// Position + Depth
	createAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &deferredFrameBuffers.normal, width, height);			// Normals
	createAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &deferredFrameBuffers.albedo, width, height);			// Albedo (color)
	createAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &deferredFrameBuffers.mix, width, height);			// mix
	createAttachment(attDepthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &deferredFrameBuffers.depth, width, height);			// Depth

	// Render passes

	// G-Buffer creation
	{
		std::array<VkAttachmentDescription, 5> attachmentDescs = {};

		// Init attachment properties
		for (uint32_t i = 0; i < static_cast<uint32_t>(attachmentDescs.size()); i++)
		{
			attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDescs[i].finalLayout = (i == 4) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		// Formats
		attachmentDescs[0].format = deferredFrameBuffers.position.format;
		attachmentDescs[1].format = deferredFrameBuffers.normal.format;
		attachmentDescs[2].format = deferredFrameBuffers.albedo.format;
		attachmentDescs[3].format = deferredFrameBuffers.mix.format;
		attachmentDescs[4].format = deferredFrameBuffers.depth.format;

		std::vector<VkAttachmentReference> colorReferences;
		colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		colorReferences.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		colorReferences.push_back({ 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		VkAttachmentReference depthReference = {};
		depthReference.attachment = 4;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pColorAttachments = colorReferences.data();
		subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
		subpass.pDepthStencilAttachment = &depthReference;
		
		// Use subpass dependencies for attachment layout transitions
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
		renderPassInfo.pAttachments = attachmentDescs.data();
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = dependencies.data();
		VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));

		std::array<VkImageView, 5> attachments;
		attachments[0] = deferredFrameBuffers.position.view;
		attachments[1] = deferredFrameBuffers.normal.view;
		attachments[2] = deferredFrameBuffers.albedo.view;
		attachments[3] = deferredFrameBuffers.mix.view;
		attachments[4] = deferredFrameBuffers.depth.view;

		VkFramebufferCreateInfo fbufCreateInfo = vks::initializers::framebufferCreateInfo();
		fbufCreateInfo.renderPass = renderPass;
		fbufCreateInfo.pAttachments = attachments.data();
		fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		fbufCreateInfo.width = deferredFrameBuffers.width;
		fbufCreateInfo.height = deferredFrameBuffers.height;
		fbufCreateInfo.layers = 1;
		VK_CHECK_RESULT(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &deferredFrameBuffers.frameBuffer));
	}

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
void GbufferPass::createPipeline(VkPipelineVertexInputStateCreateInfo &vertexInputState)
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;


	VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(pipelineLayout, renderPass, 0);
	pipelineCreateInfo.pVertexInputState = &vertexInputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();

	// Blend attachment states required for all color attachments
	// This is important, as color write mask will otherwise be 0x0 and you
	// won't see anything rendered to the attachment
	std::array<VkPipelineColorBlendAttachmentState, 4> blendAttachmentStates = {
		vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE)
	};
	colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
	colorBlendState.pAttachments = blendAttachmentStates.data();

	shaderStages[0] = loadShader(getAssetPath + std::string("gbuffer/gbuffer.vert.spv"), VK_SHADER_STAGE_VERTEX_BIT, device, shaderModules);
	shaderStages[1] = loadShader(getAssetPath + std::string("gbuffer/gbuffer.frag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT, device, shaderModules);

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, 0, 1, &pipelineCreateInfo, nullptr, &pipeline));
	
}
void GbufferPass::createDescriptorsLayouts(VkDescriptorPool &descriptorPool)
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
	VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo;
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo();

	// G-Buffer creation 
	setLayoutBindings = {
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),								// VS UBO
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),	
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 5),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 6),
	};
	setLayoutCreateInfo = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), static_cast<uint32_t>(setLayoutBindings.size()));
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &setLayoutCreateInfo, nullptr, &descriptorSetLayout));

	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));


}
void GbufferPass::wirteDescriptorSets(VkDescriptorPool &descriptorPool, std::vector<VkDescriptorImageInfo> &texDescriptor)
{
	VkDescriptorSetAllocateInfo descriptorAllocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, nullptr, 1);
	std::vector<VkWriteDescriptorSet> writeDescriptorSets;
	descriptorAllocInfo.pSetLayouts = &descriptorSetLayout;
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorAllocInfo, &descriptorSet));
	writeDescriptorSets = {
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers.sceneMatrices.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1 , &texDescriptor[0]),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2 , &texDescriptor[1]),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,3 , &texDescriptor[2]),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,4 , &texDescriptor[3]),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,5 , &texDescriptor[4]),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,6 , &texDescriptor[5]),
	};
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);


}
void GbufferPass::buildCommandBuffer(VkCommandPool cmdPool,VkBuffer vertexBuffer,VkBuffer indexBuffer,uint32_t indexCount)
{
	VkDeviceSize offsets[1] = { 0 };
	cmdBuffer = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, false, cmdPool);

	// Create a semaphore used to synchronize offscreen rendering and usage
	VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
	VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore));

	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	// Clear values for all attachments written in the fragment sahder
	std::vector<VkClearValue> clearValues(5);
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[3].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[4].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass =renderPass;
	renderPassBeginInfo.framebuffer = deferredFrameBuffers.frameBuffer;
	renderPassBeginInfo.renderArea.extent.width = deferredFrameBuffers.width;
	renderPassBeginInfo.renderArea.extent.height = deferredFrameBuffers.height;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));

	/*
		First pass: Fill G-Buffer components (positions+depth, normals, albedo) using MRT
	*/

	vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = vks::initializers::viewport((float)deferredFrameBuffers.width, (float)deferredFrameBuffers.height, 0.0f, 1.0f);
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

	VkRect2D scissor = vks::initializers::rect2D(deferredFrameBuffers.width, deferredFrameBuffers.height, 0, 0);
	vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &vertexBuffer, offsets);
	vkCmdBindIndexBuffer(cmdBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(cmdBuffer, indexCount, 1, 0, 0, 0);
	vkCmdEndRenderPass(cmdBuffer);
	VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));
}
void GbufferPass::createUniformBuffers(VkQueue queue, glm::mat4 &perspective, glm::mat4 &view, glm::mat4 &lightSpace)
{
	// Scene matrices
	vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformBuffers.sceneMatrices,
		sizeof(uboSceneMatrices));

	updateUniformBufferMatrices(perspective, view,lightSpace);

}
void GbufferPass::updateUniformBufferMatrices(glm::mat4 &perspective, glm::mat4 &view,glm::mat4 &lightSpace)
{
	uboSceneMatrices.projection = perspective;
	uboSceneMatrices.view = view;
	uboSceneMatrices.model = glm::mat4(1.0f);
	uboSceneMatrices.lightSpace = lightSpace;
	VK_CHECK_RESULT(uniformBuffers.sceneMatrices.map());
	uniformBuffers.sceneMatrices.copyTo(&uboSceneMatrices, sizeof(uboSceneMatrices));
	uniformBuffers.sceneMatrices.unmap();
}