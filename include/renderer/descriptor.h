#pragma once
#include "vulkan/vulkan.h"
#include "VulkanDevice.hpp"
#include "VulkanTools.h"
#include "VulkanInitializers.hpp"
#include "VulkanTexture.hpp"
#include "bobject/material.h"
class Descriptor
{
public:
	Descriptor(vks::VulkanDevice *vulkanDevice);

	~Descriptor();

	void setupDescriptorSetLayout(VkPipelineLayout &shaded);

	void setupDescriptorSet();
	void setupShadowMapSet();
	void updateDescriptorSet(vks::TextureCubeMap &IrradianceMap, vks::TextureCubeMap &prefilteredCube, vks::Texture2D &lutMap);
	void createShadowMapTexDescriptor(VkSampler depthSampler, VkImageView view);
	VkDescriptorImageInfo shadowMapTexDescriptor;
	vks::Texture2D colorMap;
	
	vks::TextureCubeMap cubeMap;
	struct {
	vks::Texture2D albedoMap;
	vks::Texture2D normalMap;
	vks::Texture2D aoMap;
	vks::Texture2D metallicMap;
	vks::Texture2D roughnessMap;
	} pbrTex;

	struct {
		vks::Buffer object;
		vks::Buffer params;
		vks::Buffer skybox;
		vks::Buffer shadowMapMvp;
	} uniformBuffers;

	struct UBOVS {
		glm::mat4 projection;
		glm::mat4 model;
		float lodBias = 0.0f;
	} uboVS;

	struct UBOMatrices {
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 depthBiasMVP;
		glm::vec3 camPos;
	} uboMatrices;

	struct UBOParams {
		glm::vec4 lights[4];
	} uboParams;

	struct {
		glm::mat4 depthMVP;
	} uboShadowMapMvpVS;

	VkDescriptorSetLayout descriptorSetLayout;
	
	std::vector<VkDescriptorSetLayout> set_layouts;
	VkDescriptorSet descriptorSet;
	VkDescriptorSet descriptorSetSkybox;
	VkDescriptorSet descriptorSetShadowMap;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;


private:

	void _setupDescriptorPool();
	void _createUniformBuffers();
	vks::VulkanDevice * vulkanDevice;
	VkDevice device;

};

