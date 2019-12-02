#include "renderer/descriptor.h"

Descriptor::Descriptor(vks::VulkanDevice *vulkanDevice)
{
	this->vulkanDevice = vulkanDevice;
	this->device = vulkanDevice->logicalDevice;	
}

Descriptor::~Descriptor()
{
}

void Descriptor::_setupDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 16),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 18)
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo =
		vks::initializers::descriptorPoolCreateInfo(
			poolSizes.size(),
			poolSizes.data(),
			128);

	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
}

void Descriptor::setupDescriptorSetLayout(VkPipelineLayout &shaded)
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
	VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo;
	

	// Binding 0 : Vertex shader uniform buffer
	setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		0));

	// Binding 1 : frag shader uniform buffer
	setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		1));
	// Binding 2:  frag shader uniform buffer
	setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		2));
	setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		3));
	setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		4));
	setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		5));
	setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		6));
	setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		7));
	setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		8));
	setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		9));
	setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		10));

	descriptorLayoutInfo = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayoutInfo, nullptr, &descriptorSetLayout));


	set_layouts.push_back(descriptorSetLayout);
	set_layouts.push_back(descriptorSetLayout);
	pipelineLayoutInfo = vks::initializers::pipelineLayoutCreateInfo(set_layouts.data(), set_layouts.size());

    
	std::vector<VkPushConstantRange> pushConstantRanges = {
	vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::vec3), 0),
	vks::initializers::pushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(MaterialPbr::PushBlock), sizeof(glm::vec3)),
	};

	// Push constant ranges are part of the pipeline layout
	
	pipelineLayoutInfo.pushConstantRangeCount = 2;
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &shaded));

	_setupDescriptorPool();
	_createUniformBuffers();
	setupDescriptorSet();
}

void Descriptor::_createUniformBuffers()
{

	// Objact vertex shader uniform buffer
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformBuffers.object,
		sizeof(uboMatrices)));

	// material parameter uniform buffer
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformBuffers.params,
		sizeof(uboParams)));
	// Skybox vertex shader uniform buffer
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformBuffers.skybox,
		sizeof(uboVS)));

	// shadowmap vertex shader uniform buffer block
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformBuffers.shadowMapMvp,
		sizeof(uboShadowMapMvpVS)));
	// Map persistent
	VK_CHECK_RESULT(uniformBuffers.object.map());
	VK_CHECK_RESULT(uniformBuffers.skybox.map());
	VK_CHECK_RESULT(uniformBuffers.params.map());
	VK_CHECK_RESULT(uniformBuffers.shadowMapMvp.map());
	

}
void Descriptor::setupShadowMapSet() 
{
	// Mirror plane descriptor set
	VkDescriptorSetAllocateInfo allocInfo =
		vks::initializers::descriptorSetAllocateInfo(
			descriptorPool,
			set_layouts.data(),
			set_layouts.size());
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));
	std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
		// Binding 0 : Vertex shader uniform buffer
		vks::initializers::writeDescriptorSet(
			descriptorSetShadowMap,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			0,
			&uniformBuffers.shadowMapMvp.descriptor),
	};
	vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);

}
void Descriptor::setupDescriptorSet()
{
	// Mirror plane descriptor set
	VkDescriptorSetAllocateInfo allocInfo =
		vks::initializers::descriptorSetAllocateInfo(
			descriptorPool,
			set_layouts.data(),
			set_layouts.size());

	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

	std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers.object.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &uniformBuffers.params.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &colorMap.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &cubeMap.descriptor),

		/*vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers.object.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &uniformBuffers.params.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &colorMap.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &cubeMap.descriptor),*/
	};
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSetSkybox));
	writeDescriptorSets = {
	   vks::initializers::writeDescriptorSet(descriptorSetSkybox, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers.skybox.descriptor),
	   vks::initializers::writeDescriptorSet(descriptorSetSkybox, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &uniformBuffers.params.descriptor),
	   vks::initializers::writeDescriptorSet(descriptorSetSkybox, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &colorMap.descriptor),
	   vks::initializers::writeDescriptorSet(descriptorSetSkybox, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &cubeMap.descriptor),

	};
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
	

}
void Descriptor::updateDescriptorSet(vks::TextureCubeMap &IrradianceMap, vks::TextureCubeMap &prefilteredCube,vks::Texture2D &lutMap)
{

	std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers.object.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &uniformBuffers.params.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &IrradianceMap.descriptor),
	
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &prefilteredCube.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, &lutMap.descriptor),

		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, &pbrTex.albedoMap.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, &pbrTex.normalMap.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 7, &pbrTex.aoMap.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8, &pbrTex.metallicMap.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 9, &pbrTex.roughnessMap.descriptor),
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10, &shadowMapTexDescriptor),
	};
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);


	writeDescriptorSets = {
	   vks::initializers::writeDescriptorSet(descriptorSetSkybox, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers.skybox.descriptor),
	   vks::initializers::writeDescriptorSet(descriptorSetSkybox, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &uniformBuffers.params.descriptor),
	   vks::initializers::writeDescriptorSet(descriptorSetSkybox, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &IrradianceMap.descriptor),
	   vks::initializers::writeDescriptorSet(descriptorSetSkybox, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &cubeMap.descriptor),

	};
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);

}
void Descriptor::createShadowMapTexDescriptor(VkSampler depthSampler, VkImageView view)
{
	// Image descriptor for the shadow map attachment
	shadowMapTexDescriptor =
		vks::initializers::descriptorImageInfo(
			depthSampler,
			view,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

}
