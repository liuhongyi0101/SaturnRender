#include "renderer/pipeline.h"
#include "utils/loadshader.h"
Pipeline::Pipeline(VkDevice device)
{
	createPipelineCache(device);
}

Pipeline::~Pipeline()
{
	//vkDestroyPipelineCache(device, pipelineCache, nullptr);
}
void Pipeline::createPipelineCache(VkDevice device)
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VK_CHECK_RESULT(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache));
}
void Pipeline::createShadowPipeline(VkDevice device,VkRenderPass renderPass, VkPipelineLayout pipelineLayout, std::shared_ptr<VertexDescriptions> vdo)
{
	pipelineCreateInfo =
		vks::initializers::pipelineCreateInfo(pipelineLayout, renderPass);
	depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(1, 1, VK_COMPARE_OP_LESS_OR_EQUAL);
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pVertexInputState = &vdo->vertices.inputState;

	// No blend attachment states(no color attachments used)
	rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
	colorBlendState.attachmentCount = 0;
	// Cull front faces
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	// Enable depth bias
	rasterizationState.depthBiasEnable = VK_TRUE;
	// Add depth bias to dynamic state, so we can change it at runtime
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
	dynamicState =
		vks::initializers::pipelineDynamicStateCreateInfo(
			dynamicStateEnables.data(),
			dynamicStateEnables.size(),
			0);

	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	createPipeline(device, std::string("shadowmapping/depth.vert.spv"), std::string("shadowmapping/depth.frag.spv"), depthstate, pipelines["genShadowPipeline"]);
	
}
void Pipeline::createQuadPipeline(VkDevice device, VkRenderPass renderPass, VkPipelineLayout pipelineLayout,std::string pipelinename)
{
	// out pass
	depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(0, 0, VK_COMPARE_OP_NEVER);
	rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
	colorBlendState =
		vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	pipelineCreateInfo.pVertexInputState = &emptyInputState;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	shaderStages[0] = loadShader(getAssetPath + "ssao/fullscreen.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, device, shaderModules);
	shaderStages[1] = loadShader(getAssetPath + "ssao/composition.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, device, shaderModules);


	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, 0, 1, &pipelineCreateInfo, nullptr, &pipelines[pipelinename]));
}
void Pipeline::createGbufferPipeline(VkDevice device, VkRenderPass renderPass, VkPipelineLayout pipelineLayout)
{
	
		pipelineCreateInfo.renderPass = renderPass;
		pipelineCreateInfo.layout = pipelineLayout;
		// Blend attachment states required for all color attachments
		// This is important, as color write mask will otherwise be 0x0 and you
		// won't see anything rendered to the attachment
		std::array<VkPipelineColorBlendAttachmentState, 3> blendAttachmentStates = {
			vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
			vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
			vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE)
		};
		colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
		colorBlendState.pAttachments = blendAttachmentStates.data();
		createPipeline(device, std::string("ssao/gbuffer.vert.spv"), std::string("ssao/gbuffer.frag.spv"), depthstate, pipelines["gbufferPipeline"]);
}
void Pipeline::createPipeline(VkDevice device,std::string &vertshader,std::string &fragshader,std::array<int,2> &depthstate, VkPipeline &pipeline) {

	shaderStages[0] = loadShader(getAssetPath + vertshader, VK_SHADER_STAGE_VERTEX_BIT, device, shaderModules);
	shaderStages[1] = loadShader(getAssetPath + fragshader, VK_SHADER_STAGE_FRAGMENT_BIT, device, shaderModules);

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, 0, 1, &pipelineCreateInfo, nullptr, &pipeline));
}


