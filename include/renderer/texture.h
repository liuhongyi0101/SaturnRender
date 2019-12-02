#pragma once
/*
* Vulkan Example - Texture loading (and display) example (including mip maps)
*
* Copyright (C) 2016-2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/


#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gli/gli.hpp>

#include <vulkan/vulkan.h>
#include "VulkanDevice.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanTexture.hpp"


#define VERTEX_BUFFER_BIND_ID 0

// Vertex layout for this example
struct Vertex {
	glm::vec3 pos[3];
	float uv[2];
	float normal[3];
};
class Texture
{
public:
	bool animate = true;

	PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR;
	VkPhysicalDevicePushDescriptorPropertiesKHR pushDescriptorProps{};

	vks::Texture2D textureImage;
	std::vector<vks::Texture2D> textureImages;
	
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;

	VkDescriptorSet descriptorSet;
	VkPipelineCache pipelineCache;

	vks::VulkanDevice *vulkanDevice;
	VkQueue queue;
	VkRenderPass renderPass;
	VkPhysicalDeviceProperties deviceProperties;
	VkDevice device;
	VkCommandBuffer cmdBuffers;
	// Contains all Vulkan objects that are required to store and use a texture
	// Note that this repository contains a texture class (VulkanTexture.hpp) that encapsulates texture loading functionality in a class that is used in subsequent demos
	
	struct {
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} vertices;

	vks::Buffer vertexBuffer;
	vks::Buffer indexBuffer;
	uint32_t indexCount;

	Texture(vks::VulkanDevice *vulkanDevice, VkQueue queue, VkRenderPass renderPass);
	
	~Texture();

	void loadTexture(const std::string &textpath);

	void buildCommandBuffers(VkCommandBuffer cmdBuffers, uint32_t &index);
	void generateMark(glm::vec3 &marlPos);
	
	void setupVertexDescriptions();
	void setupDescriptorPool();
	void setupDescriptorSetLayout();
	void preparePipelines(const std::string &evnPath, VkSampleCountFlagBits _sampleCount);
	void prepareUniformBuffers();
	void updateUniformBuffers();
	void createTexture();
	void prepare(const std::string &textpath, VkSampleCountFlagBits _sampleCount);
	void bindPipeLine(VkCommandBuffer cmdBuffers);

private:
	std::array<VkWriteDescriptorSet, 1> writeDescriptorSets{};
	std::string path;
};


	


