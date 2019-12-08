#pragma once
#include "graphics/basePass.h"
class SsrPass :public BasePass
{
public:
	SsrPass(vks::VulkanDevice * vulkanDevice);

	void createFramebuffersAndRenderPass(uint32_t width, uint32_t  height);
	void createPipeline();
	void createDescriptorsLayouts();
	void wirteDescriptorSets(VkDescriptorPool &descriptorPool, std::vector<VkDescriptorImageInfo> &texDescriptor);
	void createUniformBuffers(VkQueue queue, glm::mat4 &invPerspective);
	void updateUniformBufferMatrices(glm::mat4 &invPerspective);
	void buildCommandBuffer(VkCommandBuffer& cmdBuffer);
	~SsrPass();


	struct SsrRtFrameBuffer : public FrameBuffer {
		FrameBufferAttachment ssrRtAttachment;
	} ssrRtFrameBuffer;

	struct UBOParams {
		glm::mat4 invProjection;
		
	
	} uboParams;

	vks::Buffer uniformBuffers;

private:

};