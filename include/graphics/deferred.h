#pragma once
#include"graphics/basePass.h"

class DeferredPass : public BasePass
{
public:
	DeferredPass(vks::VulkanDevice * vulkanDevice);
	void createFramebuffersAndRenderPass(uint32_t width, uint32_t  height);
	void createDescriptorsLayouts(VkDescriptorPool &descriptorPool);
	void createPipeline(VkPipelineVertexInputStateCreateInfo &vertexInputState);
	void createUniformBuffers(VkQueue queue, glm::mat4 &perspective, glm::mat4 &view, glm::mat4 &lightSpace);
	void wirteDescriptorSets(VkDescriptorPool &descriptorPool, VkDescriptorImageInfo &shadowMapTexDescriptor);
	void updateUniformBufferMatrices(glm::mat4 &perspective, glm::mat4 &view,glm::mat4 &lightSpace);
	void buildCommandBuffer(VkCommandPool cmdPool, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indexCount);
	
	~DeferredPass();

	struct DeferredFrameBuffer : public FrameBuffer {
			FrameBufferAttachment position, normal, albedo, mix, depth;
		} deferredFrameBuffers;

	struct UBOSceneMatrices {
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 lightSpace;
	} uboSceneMatrices;
	struct {
		vks::Buffer sceneMatrices;

	} uniformBuffers;

};