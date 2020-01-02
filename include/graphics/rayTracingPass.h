#pragma once
#include "graphics/basePass.h"
#include <array>
#include <random>
class RayTracingPass :public BasePass
{
public:
	RayTracingPass(vks::VulkanDevice * vulkanDevice);
	~RayTracingPass();
	void createFramebuffersAndRenderPass(uint32_t width, uint32_t  height);
	void createPipeline();
	void createDescriptorsLayouts(VkDescriptorPool &descriptorPool);
	void wirteDescriptorSets();
	void createUniformBuffers(VkQueue queue, glm::mat4 &invPerspective, glm::vec3 &cameraPos);
	void updateUniformBufferMatrices(glm::mat4 &invPerspective, glm::vec3 &cameraPos);
	void updateUniformBufferMatrices();
	void buildCommandBuffer(VkCommandBuffer& cmdBuffer);
	void createNoiseTex(VkQueue queue);
	std::array<VkDescriptorImageInfo, 2> pingpongDescriptor;
	int ping = 1;
	int count = 1;
private:
	struct RtFrameBuffer : public FrameBuffer {
		FrameBufferAttachment rtAttachment;
	};
	struct UBOParams {
		glm::mat4 modle;
		glm::mat4 invpv;
		glm::vec4 cameraPos;
		glm::vec2 rand;
	} uboParams;
	std::array <RtFrameBuffer,2> pingpong;

	vks::Buffer uniformBuffers;
	const int randsize = 1024;
	
	vks::Texture2D randVec2Tex;
	vks::Texture2D randVec3Tex;
	
	glm::vec3 eye = glm::vec3(0., 0., 10.0);
	std::vector<VkWriteDescriptorSet> writeDescriptorSets;
	void updateCommandBuffer(VkCommandBuffer &cmdBuffer);
	
};