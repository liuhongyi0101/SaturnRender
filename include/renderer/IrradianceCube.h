#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <gli/gli.hpp>


#include "VulkanDevice.hpp"
#include "VulkanTexture.hpp"
#include "VulkanModel.hpp"
#include "renderer/vertexDescriptions.h"
#include "utils/loadasset.h"


class IrradianceCube
{
public:
	IrradianceCube(vks::VulkanDevice *vulkanDevice, VkCommandPool &cmdPool,std::shared_ptr<VertexDescriptions> &vdo_, Meshes &models, VkQueue &queue);
	~IrradianceCube();
	void generateIrradianceCube(vks::TextureCubeMap &cubeMap);
	void generatePrefilteredCube(vks::TextureCubeMap &cubeMap);
	void generateBRDFLUT(vks::TextureCubeMap &cubeMap);
	VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin);
	void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free);
	VkDevice device;
	vks::VulkanDevice *vulkanDevice;
	VkCommandPool cmdPool;
	std::vector<VkShaderModule> shaderModules;
	std::shared_ptr<VertexDescriptions> vdo_;
	Meshes models;
	VkQueue queue;
	struct Textures {
	
		// Generated at runtime
		vks::Texture2D lutBrdf;
		vks::TextureCubeMap irradianceCube;
		vks::TextureCubeMap prefilteredCube;
	};
	Textures textures;
private:

};