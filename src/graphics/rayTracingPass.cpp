#include "graphics/rayTracingPass.h"
#include "utils/loadshader.h"
#include <ctime>
RayTracingPass::RayTracingPass(vks::VulkanDevice * vulkanDevice)
{
	this->vulkanDevice = vulkanDevice;
	this->device = vulkanDevice->logicalDevice;
}

RayTracingPass::~RayTracingPass()
{
}
void RayTracingPass::createNoiseTex(VkQueue queue)
{
	const int width = 1280;
	const int heigt = 720;
	std::default_random_engine rndEngine(0);
	std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);
	std::vector<glm::vec4> noise(width * heigt);
	for (uint32_t i = 0; i < static_cast<uint32_t>(noise.size()); i++)
	{
		noise[i] = glm::vec4((rand() % 10) / 10., (rand() % 10) / 10., 2.0*((rand() % 10) / 10.) -1.0 , 2.0*((rand() % 10) / 10.) - 1.0);
	}
	randVec2Tex.fromBuffer(noise.data(), noise.size() * sizeof(glm::vec4), VK_FORMAT_R32G32B32A32_SFLOAT, width, heigt, vulkanDevice, queue, VK_FILTER_NEAREST);


	std::vector<glm::vec3> noiseVec3(width * heigt);
	for (uint32_t i = 0; i < static_cast<uint32_t>(noiseVec3.size()); i++)
	{
		noiseVec3[i] = glm::vec3(2.0*((rand() % 10) / 10.) - 1.0, 2.0*((rand() % 10) / 10.) - 1.0, 2.0*((rand() % 10) / 10.) - 1.0);
	}
	randVec3Tex.fromBuffer(noiseVec3.data(), noiseVec3.size() * sizeof(glm::vec3), VK_FORMAT_R32G32B32_SFLOAT, width, heigt, vulkanDevice, queue, VK_FILTER_NEAREST);
}

void RayTracingPass::createFramebuffersAndRenderPass(uint32_t width, uint32_t  height)
{

	const uint32_t ssaoWidth = width;
	const uint32_t ssaoHeight = height;

	pingpong[ping].setSize(width, height);
	pingpong[ping-1].setSize(width, height);

	createAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &pingpong[ping].rtAttachment, width, height);
	createAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &pingpong[ping-1].rtAttachment, width, height);
	// Render passes

	{
		VkAttachmentDescription attachmentDescs = {};

		// Init attachment properties

		attachmentDescs.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescs.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescs.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescs.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescs.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


		// Formats
		attachmentDescs.format = pingpong[ping].rtAttachment.format;


		std::vector<VkAttachmentReference> colorReferences;
		colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pColorAttachments = colorReferences.data();
		subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());


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
		renderPassInfo.pAttachments = &attachmentDescs;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = dependencies.data();
		VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));

		VkImageView attachments;
		attachments = pingpong[ping].rtAttachment.view;


		VkFramebufferCreateInfo fbufCreateInfo = vks::initializers::framebufferCreateInfo();
		fbufCreateInfo.renderPass = renderPass;
		fbufCreateInfo.pAttachments = &attachments;
		fbufCreateInfo.attachmentCount = 1;
		fbufCreateInfo.width = pingpong[ping].width;
		fbufCreateInfo.height = pingpong[ping].height;
		fbufCreateInfo.layers = 1;

		VK_CHECK_RESULT(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &pingpong[ping].frameBuffer));
		fbufCreateInfo.pAttachments = &pingpong[ping-1].rtAttachment.view;
		VK_CHECK_RESULT(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &pingpong[ping-1].frameBuffer));
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

	pingpongDescriptor[ping] = vks::initializers::descriptorImageInfo(colorSampler, pingpong[ping].rtAttachment.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	pingpongDescriptor[ping-1] = vks::initializers::descriptorImageInfo(colorSampler, pingpong[ping-1].rtAttachment.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void RayTracingPass::createDescriptorsLayouts(VkDescriptorPool &descriptorPool)
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
	VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo;
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo();


	setLayoutBindings = {
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
		
	};
	setLayoutCreateInfo = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), static_cast<uint32_t>(setLayoutBindings.size()));
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &setLayoutCreateInfo, nullptr, &descriptorSetLayout));

	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));


	VkDescriptorSetAllocateInfo descriptorAllocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, nullptr, 1);

	descriptorAllocInfo.pSetLayouts = &descriptorSetLayout;
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorAllocInfo, &descriptorSet));
}
void RayTracingPass::createPipeline()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
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

	shaderStages[0] = loadShader(getAssetPath + std::string("ssr/ssr.vert.spv"), VK_SHADER_STAGE_VERTEX_BIT, device, shaderModules);
	shaderStages[1] = loadShader(getAssetPath + std::string("rayTracing/ray.frag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT, device, shaderModules);

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, 0, 1, &pipelineCreateInfo, nullptr, &pipeline));

}
void RayTracingPass::wirteDescriptorSets()
{
	writeDescriptorSets = {
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1 , &pingpongDescriptor[1-ping]),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2 , &randVec2Tex.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,3 , &randVec3Tex.descriptor),
	};
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
}
void RayTracingPass::createUniformBuffers(VkQueue queue, glm::mat4 &invPerspective, glm::vec3 &cameraPos)
{
	// Scene matrices
	vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformBuffers,
		sizeof(uboParams));
	uboParams.modle = glm::mat4(1.0);

	glm::mat4 view = glm::lookAt( eye, glm::vec3(0.0, 0.0, 0.),glm::vec3(0.0,1.0,0.0));
	
	glm::mat4 pro = glm::perspective(glm::radians(60.f), 1280.0f/ 720.0f, 0.1f, 1000.f);
	
	glm::mat4 inpv = pro * view;
	updateUniformBufferMatrices(inpv, eye);
}

void RayTracingPass::updateUniformBufferMatrices(glm::mat4 &invPerspective, glm::vec3 &cameraPos)
{
	std::default_random_engine rndEngine(time(0));
	std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);
	glm::mat4 invp = glm::inverse(invPerspective);

	uboParams.invpv = invp;
	uboParams.cameraPos = glm::vec4(cameraPos,1.0);

	uboParams.rand = glm::vec2(rndDist(rndEngine), rndDist(rndEngine));
	VK_CHECK_RESULT(uniformBuffers.map());
	uniformBuffers.copyTo(&uboParams, sizeof(uboParams));
	uniformBuffers.unmap();
}
void RayTracingPass::updateUniformBufferMatrices()
{
	std::default_random_engine rndEngine(time(0));
	std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);
	uboParams.rand = glm::vec2(rndDist(rndEngine), rndDist(rndEngine));
	VK_CHECK_RESULT(uniformBuffers.map());
	uniformBuffers.copyTo(&uboParams, sizeof(uboParams));
	uniformBuffers.unmap();
}

void RayTracingPass::buildCommandBuffer(VkCommandBuffer &cmdBuffer)
{
	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	std::vector<VkClearValue> clearValues(1);
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = pingpong[ping].frameBuffer;
	renderPassBeginInfo.renderArea.extent.width = pingpong[ping].width;
	renderPassBeginInfo.renderArea.extent.height = pingpong[ping].height;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = vks::initializers::viewport((float)pingpong[ping].width, (float)pingpong[ping].height, 0.0f, 1.0f);
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

	VkRect2D scissor = vks::initializers::rect2D(pingpong[ping].width, pingpong[ping].height, 0, 0);
	vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

	vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
	vkCmdEndRenderPass(cmdBuffer);

}

void RayTracingPass::updateCommandBuffer(VkCommandBuffer &cmdBuffer)
{




}