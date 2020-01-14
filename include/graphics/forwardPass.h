#pragma once
#include "basePass.h"
class ForwardPass :public BasePass
{
public:
	ForwardPass(vks::VulkanDevice * vulkanDevice);
	~ForwardPass();

	void createFramebuffersAndRenderPass(uint32_t width, uint32_t  height);
	void createDescriptorsLayouts(VkDescriptorPool &descriptorPool);
	void createPipeline(VkPipelineVertexInputStateCreateInfo &vertexInputState);
	void createUniformBuffers(VkQueue queue, glm::mat4 &perspective, glm::mat4 &view, glm::vec3 &lightDir);
	void wirteDescriptorSets(VkDescriptorPool &descriptorPool, std::vector<VkDescriptorImageInfo> &texDescriptor);
	void updateUniformBufferMatrices(glm::mat4 &perspective, glm::mat4 &view, glm::vec3 &lightDir);
	void buildCommandBuffer(VkCommandPool cmdPool, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indexCount);

	struct ForwardFrameBuffer : public FrameBuffer {
		FrameBufferAttachment color, depth;
	} ;
	std::array<ForwardFrameBuffer, 2> pingpongRT;
	
	struct UBOSceneMatrices {
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;
		glm::vec3 lightDir;
	} uboSceneMatrices;
	struct {
		vks::Buffer sceneMatrices;

	} uniformBuffers;

private:
	int ping = 0;

};