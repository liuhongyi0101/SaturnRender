#pragma once
#include "vulkan/vulkan.h"
#include "VulkanDevice.hpp"
#include "VulkanTools.h"
#include "VulkanInitializers.hpp"
#include "VulkanTexture.hpp"
#include "VulkanModel.hpp"
#include "renderer/pipeline.h"

#define SSAO_KERNEL_SIZE 64
#define SSAO_RADIUS 0.5f
#define SSAO_NOISE_DIM 4

class SsaoPass
{
public:
	SsaoPass(vks::VulkanDevice * vulkanDevice, uint32_t  width, uint32_t   height);
	~SsaoPass();
#pragma region Field
	vks::VulkanDevice * vulkanDevice;
	VkDevice device;
	struct {
		vks::Texture2D ssaoNoise;
	} textures;
	struct UBOSSAOParams {
		glm::mat4 projection;
	} uboSSAOParams;

	struct {
;
		VkPipeline ssao;
		VkPipeline ssaoBlur;
	} pipelines;

	struct {
		
		VkPipelineLayout ssao;
		VkPipelineLayout ssaoBlur;
		
	} pipelineLayouts;

	struct {
		const uint32_t count = 5;
		VkDescriptorSet ssao;
		VkDescriptorSet ssaoBlur;
	
	} descriptorSets;

	struct {
		VkDescriptorSetLayout ssao;
		VkDescriptorSetLayout ssaoBlur;
	} descriptorSetLayouts;

	struct {

		vks::Buffer ssaoKernel;
		vks::Buffer ssaoParams;
	} uniformBuffers;

	// Framebuffer for offscreen rendering
	struct FrameBufferAttachment {
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
		VkFormat format;
		void destroy(VkDevice device)
		{
			vkDestroyImage(device, image, nullptr);
			vkDestroyImageView(device, view, nullptr);
			vkFreeMemory(device, mem, nullptr);
		}
	};
	struct FrameBuffer {
		int32_t width, height;
		VkFramebuffer frameBuffer;
		VkRenderPass renderPass;
		void setSize(int32_t w, int32_t h)
		{
			this->width = w;
			this->height = h;
		}
		void destroy(VkDevice device)
		{
			vkDestroyFramebuffer(device, frameBuffer, nullptr);
			vkDestroyRenderPass(device, renderPass, nullptr);
		}
	};

	struct {
		
		struct SSAO : public FrameBuffer {
			FrameBufferAttachment color;
		} ssao, ssaoBlur;
	} frameBuffers;
	// One sampler for the frame buffer color attachments
	VkSampler colorSampler;


	// Semaphore used to synchronize between offscreen and final scene rendering
	VkSemaphore semaphore = VK_NULL_HANDLE;
	VkCommandPool cmdPool;
	VkDescriptorPool descriptorPool;
	std::vector<VkShaderModule> shaderModules;
#pragma endregion

#pragma region function	
	void createAttachment(
		VkFormat format,
		VkImageUsageFlagBits usage,
		FrameBufferAttachment *attachment,
		uint32_t width,
		uint32_t height);
	void prepareFramebuffers(uint32_t  width, uint32_t   height);
	void setupLayoutsAndDescriptors(VkImageView positionView, VkImageView normalView, VkImageView colorView);
	void setupDescriptorPool();
	void buildCommandBuffer(VkCommandBuffer &cmdBuffer);
	void prepareUniformBuffers(VkQueue queue, glm::mat4 &perspective);
	void updateUniformBufferSSAOParams(glm::mat4 &perspectiv);

	void preparePipelines(std::shared_ptr<VertexDescriptions> vdo);

#pragma endregion
};