#pragma once
#include "graphics/basePass.h"
class TemporalPass :public BasePass
{
public:
	TemporalPass(vks::VulkanDevice * vulkanDevice);
	~TemporalPass();

	void createFramebuffersAndRenderPass(uint32_t width, uint32_t  height);
	void createPipeline();
	void createDescriptorsLayouts(VkDescriptorPool &descriptorPool);
	void wirteDescriptorSets();
	void createUniformBuffers(VkQueue queue, glm::mat4 &invPerspective, glm::vec3 &cameraPos);
	void updateUniformBufferMatrices(glm::mat4 &invPerspective);
	
	void buildCommandBuffer(VkCommandBuffer& cmdBuffer);
	std::array<VkDescriptorImageInfo, 2> pingpongDescriptor;
	int ping = 1;
	int count = 1;
private:
	struct RtFrameBuffer : public FrameBuffer {
		FrameBufferAttachment rtAttachment;
	};
	struct UBOParams {
		glm::mat4 invpv;
	} uboParams;
	std::array <RtFrameBuffer, 2> pingpong;

	vks::Buffer uniformBuffers;
	
	std::vector<VkWriteDescriptorSet> writeDescriptorSets;

};

