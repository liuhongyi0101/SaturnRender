#pragma once
#include "graphics/basePass.h"
class SkyboxPass :public BasePass
{
public:
	SkyboxPass(vks::VulkanDevice * vulkanDevice);
	void createModel(VkQueue queue, vks::VertexLayout &vertexLayout);
	~SkyboxPass();
	
	void createDescriptorsLayouts();
	void createPipeline(VkPipelineVertexInputStateCreateInfo &vertexInputState, VkRenderPass &renderPass);
	void createUniformBuffers(VkQueue queue, glm::mat4 &perspective, glm::mat4 &view);
	void wirteDescriptorSets(VkDescriptorPool &descriptorPool, VkDescriptorImageInfo &skyBoxMapTexDescriptor);
	void buildCommandBuffer(VkCommandBuffer &drawCmdBuffers);
	void updateUniformBuffer(glm::mat4 &perspective, glm::mat4 &view);
private:
	vks::Model skyboxModel;
	struct UBOVS {
		glm::mat4 projection;
		glm::mat4 model;
	} uboVS;
	vks::Buffer uniformSkybox;
};