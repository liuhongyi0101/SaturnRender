#pragma once
#include"graphics/basePass.h"

class DeferredShading : public BasePass
{
public:
	DeferredShading(vks::VulkanDevice * vulkanDevice);
	~DeferredShading();

	void createRenderPass(uint32_t width, uint32_t  height);
	void createFrameBuffer();
	void createPipeline();
	void createDescriptorsLayouts();
	void wirteDescriptorSets(VkDescriptorPool &descriptorPool, std::vector<VkDescriptorImageInfo> &texDescriptor, std::vector<VkDescriptorImageInfo> &iblTexDescriptor);
	void createUniformBuffers(VkQueue queue, glm::vec4 &lightPos);
	void updateUniformBufferMatrices(glm::vec4 &lightPos);
	void buildCommandBuffer(VkCommandBuffer& cmdBuffer);

	struct DeferredShadingRtFrameBuffer : public FrameBuffer {
		FrameBufferAttachment deferredShadingRtAttachment,deferredDepthRtAttachment;
	} deferredShadingRtFrameBuffer;

	struct UBOParams {
	
		glm::vec4 lightPos;

	} uboParams;

	vks::Buffer uniformBuffers;
private:

};

