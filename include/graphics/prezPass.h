#pragma once
#include "graphics/basePass.h"
class PrezPass :public BasePass
{
public:
	PrezPass(vks::VulkanDevice * vulkanDevice);
	~PrezPass();
	void createRenderPass(uint32_t width, uint32_t  height);
	void createFrameBuffer();
	void createDescriptorsLayouts();
	void createPipeline(VkPipelineVertexInputStateCreateInfo &vertexInputState);
	void createUniformBuffers(VkQueue queue, glm::mat4 &mvp);
	void wirteDescriptorSets(VkDescriptorPool &descriptorPool);
	void updateUniformBufferMatrices(glm::mat4 &mvp);
	void buildCommandBuffer(VkCommandBuffer& cmdBuffer, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indexCount);

	struct PrezRtFrameBuffer : public FrameBuffer {
		FrameBufferAttachment  prezRtAttachment;
	} prezRtFrameBuffer;

	struct UBOParams {

		glm::mat4 mvp;

	} uboParams;

	vks::Buffer uniformBuffers;

private:

};