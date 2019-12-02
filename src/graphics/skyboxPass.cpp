#include "graphics/skyboxPass.h"
#include "utils/loadshader.h"
SkyboxPass::SkyboxPass(vks::VulkanDevice * vulkanDevice)
{
	this->vulkanDevice = vulkanDevice;
	this->device = vulkanDevice->logicalDevice;
}

SkyboxPass::~SkyboxPass()
{
}
void SkyboxPass::createModel(VkQueue queue, vks::VertexLayout &vertexLayout)
{
	skyboxModel.loadFromFile(modelPath + "models/cube.obj", vertexLayout, 1.f, vulkanDevice, queue);
}
void SkyboxPass::createPipeline(VkPipelineVertexInputStateCreateInfo &vertexInputState, VkRenderPass &renderPass)
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(1, 0, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	// Additive blending
	/*blendAttachmentState.colorWriteMask = 0xF;
	blendAttachmentState.blendEnable = VK_TRUE;
	blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
	blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;*/

	VkGraphicsPipelineCreateInfo pipelineCreateInfo =
		vks::initializers::pipelineCreateInfo(pipelineLayout, renderPass);

	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pVertexInputState = &vertexInputState;

	shaderStages[0] = loadShader(getAssetPath + "pbrbasic/skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, device, shaderModules);
	shaderStages[1] = loadShader(getAssetPath + "pbrbasic/skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, device, shaderModules);

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, 0, 1, &pipelineCreateInfo, nullptr, &pipeline));

}

void SkyboxPass::createDescriptorsLayouts()
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
	VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo;
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo();

	setLayoutBindings = {
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),	
		
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),						// FS Color
	};
	setLayoutCreateInfo = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), static_cast<uint32_t>(setLayoutBindings.size()));
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &setLayoutCreateInfo, nullptr, &descriptorSetLayout));

	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

}
void SkyboxPass::wirteDescriptorSets(VkDescriptorPool &descriptorPool, VkDescriptorImageInfo &skyBoxMapTexDescriptor)
{
	VkDescriptorSetAllocateInfo descriptorAllocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, nullptr, 1);
	std::vector<VkWriteDescriptorSet> writeDescriptorSets;
	descriptorAllocInfo.pSetLayouts = &descriptorSetLayout;
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorAllocInfo, &descriptorSet));
	writeDescriptorSets = {
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformSkybox.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1 , &skyBoxMapTexDescriptor),
	};
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);


}
void SkyboxPass::buildCommandBuffer(VkCommandBuffer &drawCmdBuffer)
{
	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindPipeline(drawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBindDescriptorSets(drawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
	vkCmdBindVertexBuffers(drawCmdBuffer, 0, 1, &skyboxModel.vertices.buffer, offsets);
	vkCmdBindIndexBuffer(drawCmdBuffer, skyboxModel.indices.buffer, 0, VK_INDEX_TYPE_UINT32);
	
	vkCmdDrawIndexed(drawCmdBuffer, skyboxModel.indexCount, 1, 0, 0, 0);

}
void SkyboxPass::createUniformBuffers(VkQueue queue, glm::mat4 &perspective, glm::mat4 &view)
{
	// Skybox vertex shader uniform buffer
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformSkybox,
		sizeof(uboVS)));
	VK_CHECK_RESULT(uniformSkybox.map());
	updateUniformBuffer(perspective,view);
}
void SkyboxPass::updateUniformBuffer(glm::mat4 &perspective, glm::mat4 &view)
{
	uboVS.model = view;
	uboVS.projection = perspective;
	VK_CHECK_RESULT(uniformSkybox.map());
	uniformSkybox.copyTo(&uboVS, sizeof(uboVS));
	uniformSkybox.unmap();
}