#include "renderer/command.h"
#define GRID_DIM 1

#define VERTEX_BUFFER_BIND_ID 0
Command::Command(VkDevice &device)
{

	this->device = device;
}

Command::~Command()
{
}
void Command::ssaobuildCommandBuffers(renderpassUnit renderPass, std::vector<VkCommandBuffer>& drawCmdBuffers, std::vector<VkFramebuffer> &frameBuffers)
{
	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	VkClearValue clearValues[2];
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass.renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = renderPass.width;
	renderPassBeginInfo.renderArea.extent.height = renderPass.height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
	{
		// Set target frame buffer
		renderPassBeginInfo.framebuffer = frameBuffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

		vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = vks::initializers::viewport((float)renderPass.width, (float)renderPass.height, 0.0f, 1.0f);
		vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

		VkRect2D scissor = vks::initializers::rect2D(renderPass.width, renderPass.height, 0, 0);
		vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

		vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass.pipelineLayout, 0, 1, &renderPass.descriptorSet, 0, NULL);

		// Final composition pass
		vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass.pipelineFact->pipelines["outputPipeline"]);
		vkCmdDraw(drawCmdBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(drawCmdBuffers[i]);

		VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
	}
}
void Command::buildShadowMapCommandBuffer(renderpassUnit &renderPass)
{
	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	VkClearValue clearValues[1];
	clearValues[0].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass.renderPass;
	renderPassBeginInfo.framebuffer = renderPass.frameBuffer;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = renderPass.width;
	renderPassBeginInfo.renderArea.extent.height = renderPass.height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	VK_CHECK_RESULT(vkBeginCommandBuffer(renderPass.commandBuffer, &cmdBufInfo));

	VkViewport viewport = vks::initializers::viewport((float)renderPass.width, (float)renderPass.height, 0.0f, 1.0f);
	vkCmdSetViewport(renderPass.commandBuffer, 0, 1, &viewport);

	VkRect2D scissor = vks::initializers::rect2D(renderPass.width, renderPass.height, 0, 0);
	vkCmdSetScissor(renderPass.commandBuffer, 0, 1, &scissor);

	// Depth bias (and slope) are used to avoid shadowing artefacts
// Constant depth bias factor (always applied)
	float depthBiasConstant = 1.25f;
	// Slope depth bias factor, applied depending on polygon's slope
	float depthBiasSlope = 1.75f;
	// Set depth bias (aka "Polygon offset")
	// Required to avoid shadow mapping artefacts
	vkCmdSetDepthBias(
		renderPass.commandBuffer,
		depthBiasConstant,
		0.0f,
		depthBiasSlope);

	vkCmdBeginRenderPass(renderPass.commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(renderPass.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass.pipelineFact->pipelines["genShadowPipeline"]);
	vkCmdBindDescriptorSets(renderPass.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass.pipelineLayout, 0, 1, &renderPass.descriptorSet, 0, NULL);

	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(renderPass.commandBuffer, VERTEX_BUFFER_BIND_ID, 1, &renderPass.models.vertices.buffer, offsets);
	vkCmdBindIndexBuffer(renderPass.commandBuffer, renderPass.models.indices.buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(renderPass.commandBuffer, renderPass.models.indexCount, 1, 0, 0, 0);

	vkCmdEndRenderPass(renderPass.commandBuffer);

	VK_CHECK_RESULT(vkEndCommandBuffer(renderPass.commandBuffer));
}