#pragma once
#include "graphics/basePass.h"
class SsrPass :public BasePass
{
public:
	SsrPass(vks::VulkanDevice * vulkanDevice);

	void createFramebuffersAndRenderPass(uint32_t width, uint32_t  height);
	void createPipeline();
	void createDescriptorsLayouts(VkDescriptorPool &descriptorPool);
	void wirteDescriptorSets(VkDescriptorPool &descriptorPool, VkDescriptorImageInfo &posTexDescriptor, VkDescriptorImageInfo &normalTexDescriptor, VkDescriptorImageInfo &colorTexDescriptor);
	void createUniformBuffers(VkQueue queue, glm::mat4 &perspective, glm::mat4 &view, glm::mat4 &lightSpace);
	void updateUniformBufferMatrices(glm::mat4 &perspective, glm::mat4 &view, glm::mat4 &lightSpace);
	void buildCommandBuffer(VkCommandPool cmdPool);
	~SsrPass();


	struct SsrRtFrameBuffer : public FrameBuffer {
		FrameBufferAttachment ssrRtAttachment;
	} ssrRtFrameBuffer;

	struct UBOParams {
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 lightSpace;
	} uboParams;

	vks::Buffer uniformBuffers;

private:

};