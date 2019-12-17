#pragma once
#include "graphics/basePass.h"
class BloomFFT : public BasePass  
{
public:
	BloomFFT(vks::VulkanDevice * vulkanDevice);
	
	void createDescriptorsLayouts(VkDescriptorPool &descriptorPool);
	void createPipeline(VkPipelineVertexInputStateCreateInfo &vertexInputState);
	void createUniformBuffers(VkQueue queue);
	void wirteDescriptorSets(VkDescriptorPool &descriptorPool, VkDescriptorImageInfo &shadowMapTexDescriptor);
	void updateUniformBuffer(int direction);

	~BloomFFT();
	void prepareTextureTarget( uint32_t width, uint32_t height, VkCommandPool &cmdPool,VkQueue queue);
	void buildComputeCommandBuffer();
	void prepareCompute(VkDescriptorPool &descriptorPool, VkDescriptorImageInfo  &texDescriptor);
	void getComputeQueue();
	struct Compute {
		VkQueue queue;								// Separate queue for compute commands (queue family may differ from the one used for graphics)
		VkCommandPool commandPool;					// Use a separate command pool (queue family may differ from the one used for graphics)
		VkCommandBuffer commandBuffer;				// Command buffer storing the dispatch commands and barriers
		VkFence fence;								// Synchronization fence to avoid rewriting compute CB if still in use
		VkDescriptorSetLayout descriptorSetLayout;	// Compute shader binding layout
		VkDescriptorSet descriptorSet;				// Compute shader bindings
		VkDescriptorSet descriptorSetOut;				// Compute shader bindings
		VkPipelineLayout pipelineLayout;			// Layout of the compute pipeline
		std::vector<VkPipeline> pipelines;			// Compute pipelines for image filters
		int32_t pipelineIndex = 0;					// Current image filtering compute pipeline index
		uint32_t queueFamilyIndex;					// Family index of the graphics queue, used for barriers
	} compute;
	vks::Texture2D textureComputeTarget;
	vks::Texture2D textureComputeTargetOut;
private:

	struct UBOParams {

		int direction;
		float scale;
		float strength;

	} uboParams;

	vks::Buffer uniformBuffers;

};