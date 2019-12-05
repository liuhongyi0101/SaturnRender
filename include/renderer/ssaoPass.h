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
	SsaoPass(vks::VulkanDevice * vulkanDevice,VkCommandPool &cmdPool, uint32_t  width, uint32_t   height);
	~SsaoPass();


#pragma region Field
	vks::VulkanDevice * vulkanDevice;


	VkDevice device;
	struct {
		vks::Model scene;
	} models;
	
	struct {
		vks::Texture2D ssaoNoise;
	} textures;

	struct UBOSceneMatrices {
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;
	} uboSceneMatrices;
	struct UBOSSAOParams {
		glm::mat4 projection;
		glm::vec4 cameraPos;
		glm::vec4 lightPos;
	} uboSSAOParams;

	struct {
		VkPipeline offscreen;
		VkPipeline composition;
		VkPipeline ssao;
		VkPipeline ssaoBlur;
	} pipelines;

	struct {
		VkPipelineLayout gBuffer;
		VkPipelineLayout ssao;
		VkPipelineLayout ssaoBlur;
		VkPipelineLayout composition;
	} pipelineLayouts;

	struct {
		const uint32_t count = 5;
		VkDescriptorSet model;
		VkDescriptorSet floor;
		VkDescriptorSet ssao;
		VkDescriptorSet ssaoBlur;
		VkDescriptorSet composition;
	} descriptorSets;

	struct {
		VkDescriptorSetLayout gBuffer;
		VkDescriptorSetLayout ssao;
		VkDescriptorSetLayout ssaoBlur;
		VkDescriptorSetLayout composition;
	} descriptorSetLayouts;

	struct {
		vks::Buffer sceneMatrices;
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

	VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;

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
	void buildDeferredCommandBuffer(std::shared_ptr<Pipeline> pipeline);
	VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin, VkCommandPool &cmdPool);
	void prepareUniformBuffers(VkQueue queue, glm::mat4 &perspective, glm::mat4 &view, glm::vec4& cameraPos, glm::vec4& lightpos);
	void updateUniformBufferSSAOParams(glm::mat4 &perspective, glm::vec4 &cameraPos,glm::vec4 &lightPos);
	void updateUniformBufferMatrices(glm::mat4 &perspective, glm::mat4 &view);
	void preparePipelines(std::shared_ptr<VertexDescriptions> vdo, VkRenderPass renderPass);
	void compositionSet(vks::TextureCubeMap &IrradianceMap, vks::TextureCubeMap &prefilteredCube, vks::Texture2D &lutMap,
		VkImageView positionView, VkImageView normalView, VkImageView colorView);
#pragma endregion
};