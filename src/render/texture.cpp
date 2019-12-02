#include "renderer/texture.h"
#include <stb/stb_image.h>

Texture::Texture(vks::VulkanDevice *vulkanDevice, VkQueue queue, VkRenderPass renderPass)
{
	this->vulkanDevice = vulkanDevice;
	this->deviceProperties = vulkanDevice->properties;
	this->device = vulkanDevice->logicalDevice;
	this->queue = queue;
	this->renderPass = renderPass;
}

Texture::~Texture()
{
	vkDestroyPipeline(device, pipeline, nullptr);

	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

	/*vertexBuffer.destroy();
	indexBuffer.destroy();
	uniformBufferVS.destroy();*/
}

void Texture::loadTexture(const std::string &textpath)
{
	// We use the Khronos texture format (https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/) 
	std::string filename = textpath;
	// Texture data contains 4 channels (RGBA) with unnormalized 8-bit values, this is the most commonly supported format
	VkFormat format = VK_FORMAT_BC3_UNORM_BLOCK;

	// Set to true to use linear tiled images 
	// This is just for learning purposes and not suggested, as linear tiled images are pretty restricted and often only support a small set of features (e.g. no mips, etc.)
	bool forceLinearTiling = false;

	
	/*cubes[0].texture.loadFromFile(textpath + "textures/tt.ktx", format, vulkanDevice, queue);
	cubes[1].texture.loadFromFile(textpath + "textures/tt.ktx", format, vulkanDevice, queue);*/
	int texWidth, texHeight, texChannels;
	std::string path = textpath;
	path.append("textures/aa.png");
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	VkDeviceSize imageSize = texWidth * texHeight * 4;
	//textureImage.fromBuffer(pixels, imageSize, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, vulkanDevice, queue);
	

	/*if (nullptr != testImage.image_ptr)
	{
		delete[] testImage.image_ptr;
		testImage.image_ptr = nullptr;
	}
	testImage.image_ptr = new uint8_t[imageSize];
	memcpy(testImage.image_ptr, pixels, imageSize);*/
	
	
	
	//createTexture(testImage);
	
	/*if (nullptr != testImage.image_ptr)
	{
		delete[] testImage.image_ptr;
		testImage.image_ptr = nullptr;
	}*/
	}
void Texture::createTexture()
{

	int texWidth, texHeight, texChannels;

	/*path.append("textures/a.png");
	stbi_uc* pixel = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	MARKER_INFO  bufs;
	bufs.x = texWidth;
	bufs.y = texHeight;*/
	
	//uint32_t imageSize = bufs.x * bufs.y * 4;
	/*bufs.image = new char[imageSize+1];
	for (size_t i = 0; i < imageSize; i++)
	{
		bufs.image[i] = (char)pixel[i];
	}
	bufs.image[imageSize] = '\0';*/
	//int ss = bufs.image.size();//16384
	//stbi_uc * pixels = (stbi_uc *)(bufs.image.c_str());
	/*uint8_t r = bufs.image[0];
	uint8_t g = bufs.image[1];
	uint8_t b = bufs.image[2];
	uint8_t a = bufs.image[3];*/
	//textureImage.fromBuffer(pixels, imageSize, VK_FORMAT_R8G8B8A8_UNORM, bufs.x, bufs.y, vulkanDevice, queue);
	//textureImages.push_back(textureImage);
	
}

void Texture::bindPipeLine(VkCommandBuffer cmdBuffers) {
	
	vkCmdBindPipeline(cmdBuffers, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[0].dstSet = 0;
	writeDescriptorSets[0].dstBinding = 0;
	writeDescriptorSets[0].descriptorCount = 1;
	writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
}
void Texture::buildCommandBuffers(VkCommandBuffer cmdBuffers,uint32_t &index)
{
	if (index+1>textureImages.size())
	{
		return;
	}
	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(cmdBuffers, VERTEX_BUFFER_BIND_ID, 1, &vertexBuffer.buffer, offsets);
	vkCmdBindIndexBuffer(cmdBuffers, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
	writeDescriptorSets[0].pImageInfo = &textureImages[index].descriptor;
	vkCmdPushDescriptorSetKHR(cmdBuffers, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, writeDescriptorSets.size(), writeDescriptorSets.data());
	vkCmdDrawIndexed(cmdBuffers, indexCount, 1, 0, 0, 0);

}

void Texture::generateMark(glm::vec3 &marlPos)
{
	 //Setup vertices for a single uv-mapped quad made from two triangles
	/*
	   x2-------x3
	    |		|	
		| (0,0)	|
		|		|
		x1------x0
	*/
	float h = 0.5f, w = 0.5f;
	glm::vec3 crood(marlPos);
	/*glm::vec3 x0(crood.x + w, crood.y + h, crood.z);
	glm::vec3 x1(crood.x - w, crood.y + h, crood.z);
	glm::vec3 x2(crood.x - w, crood.y - h, crood.z);
	glm::vec3 x3(crood.x + w, crood.y - h, crood.z);*/
	//crood.z = 0.0;
	glm::vec3 x0(crood.x + w, crood.y , crood.z);
	glm::vec3 x1(crood.x , crood.y , crood.z);
	glm::vec3 x2(crood.x , crood.y - h, crood.z);
	glm::vec3 x3(crood.x + w, crood.y - h, crood.z);
	std::vector<Vertex> vertices =
	{
		{ { x0 },{ 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
		{ { x1 },{ 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
		{ { x2 },{ 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } },
		{ { x3 },{ 1.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } }
	};

	// Setup indices
	std::vector<uint32_t> indices = { 0,1,2, 2,3,0 };
	indexCount = static_cast<uint32_t>(indices.size());

	// Create buffers
	// For the sake of simplicity we won't stage the vertex data to the gpu memory
	// Vertex buffer
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&vertexBuffer,
		vertices.size() * sizeof(Vertex),
		vertices.data()));
	// Index buffer
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&indexBuffer,
		indices.size() * sizeof(uint32_t),
		indices.data()));
}

void Texture::setupVertexDescriptions()
{
	// Binding description
	vertices.bindingDescriptions.resize(1);
	vertices.bindingDescriptions[0] =
		vks::initializers::vertexInputBindingDescription(
			VERTEX_BUFFER_BIND_ID,
			sizeof(Vertex),
			VK_VERTEX_INPUT_RATE_VERTEX);

	// Attribute descriptions
	// Describes memory layout and shader positions
	vertices.attributeDescriptions.resize(3);
	// Location 0 : Position
	vertices.attributeDescriptions[0] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			offsetof(Vertex, pos));
	// Location 1 : Texture coordinates
	vertices.attributeDescriptions[1] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			1,
			VK_FORMAT_R32G32_SFLOAT,
			offsetof(Vertex, uv));
	// Location 1 : Vertex normal
	vertices.attributeDescriptions[2] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			2,
			VK_FORMAT_R32G32B32_SFLOAT,
			offsetof(Vertex, normal));

	vertices.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
	vertices.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertices.bindingDescriptions.size());
	vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
	vertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertices.attributeDescriptions.size());
	vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();
}

void Texture::setupDescriptorPool()
{
	// Example uses one ubo and one image sampler
	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo =
		vks::initializers::descriptorPoolCreateInfo(
			static_cast<uint32_t>(poolSizes.size()),
			poolSizes.data(),
			2);

	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
}

void Texture::setupDescriptorSetLayout()
{
	
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		/*vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1),*/
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
	};

	VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
	descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	// Setting this flag tells the descriptor set layouts that no actual descriptor sets are allocated but instead pushed at command buffer creation time
	descriptorLayoutCI.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
	descriptorLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
	descriptorLayoutCI.pBindings = setLayoutBindings.data();
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayoutCI, nullptr, &descriptorSetLayout));

	VkPipelineLayoutCreateInfo pipelineLayoutCI = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelineLayout));


}

//void Texture::setupDescriptorSet()
//{
//	VkDescriptorSetAllocateInfo allocInfo =
//		vks::initializers::descriptorSetAllocateInfo(
//			descriptorPool,
//			&descriptorSetLayout,
//			1);
//
//	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));
//
//	// Setup a descriptor image info for the current texture to be used as a combined image sampler
//	VkDescriptorImageInfo textureDescriptor;
//	textureDescriptor.imageView = texture.view;				// The image's view (images are never directly accessed by the shader, but rather through views defining subresources)
//	textureDescriptor.sampler = texture.sampler;			// The sampler (Telling the pipeline how to sample the texture, including repeat, border, etc.)
//	textureDescriptor.imageLayout = texture.imageLayout;	// The current layout of the image (Note: Should always fit the actual use, e.g. shader read)
//
//	std::vector<VkWriteDescriptorSet> writeDescriptorSets =
//	{
//		// Binding 0 : Vertex shader uniform buffer
//		vks::initializers::writeDescriptorSet(
//			descriptorSet,
//			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//			0,
//			&uniformBufferVS.descriptor),
//		// Binding 1 : Fragment shader texture sampler
//		//	Fragment shader: layout (binding = 1) uniform sampler2D samplerColor;
//		vks::initializers::writeDescriptorSet(
//			descriptorSet,
//			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,		// The descriptor set will use a combined image sampler (sampler and image could be split)
//			1,												// Shader binding point 1
//			&textureDescriptor)								// Pointer to the descriptor image for our texture
//	};
//
//	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
//}

void Texture::preparePipelines(const std::string &evnPath, VkSampleCountFlagBits _sampleCount)
{

	const std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationStateCI = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendStateCI = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportStateCI = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleStateCI = vks::initializers::pipelineMultisampleStateCreateInfo(_sampleCount, 0);
	VkPipelineDynamicStateCreateInfo dynamicStateCI = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables.data(), static_cast<uint32_t>(dynamicStateEnables.size()), 0);

	/*blendAttachmentState.blendEnable = VK_TRUE;
	blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;*/
	
	VkGraphicsPipelineCreateInfo pipelineCreateInfoCI = vks::initializers::pipelineCreateInfo(pipelineLayout, renderPass, 0);
	pipelineCreateInfoCI.pVertexInputState = &vertices.inputState;
	pipelineCreateInfoCI.pInputAssemblyState = &inputAssemblyStateCI;
	pipelineCreateInfoCI.pRasterizationState = &rasterizationStateCI;
	pipelineCreateInfoCI.pColorBlendState = &colorBlendStateCI;
	pipelineCreateInfoCI.pMultisampleState = &multisampleStateCI;
	pipelineCreateInfoCI.pViewportState = &viewportStateCI;
	pipelineCreateInfoCI.pDepthStencilState = &depthStencilStateCI;
	pipelineCreateInfoCI.pDynamicState = &dynamicStateCI;

	



	multisampleStateCI.sampleShadingEnable = VK_TRUE;
	multisampleStateCI.minSampleShading = 0.25f;
	multisampleStateCI.rasterizationSamples = _sampleCount;
	// Pipeline cache
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VK_CHECK_RESULT(vkCreatePipelineCache(vulkanDevice->logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache));
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfoCI, nullptr, &pipeline));
}

// Prepare and initialize uniform buffer containing shader uniforms
void Texture::prepareUniformBuffers()
{// Vertex shader scene uniform buffer block
	//VK_CHECK_RESULT(vulkanDevice->createBuffer(
	//	VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	//	&uniformBuffers.scene,
	//	sizeof(UboScene)));
	//VK_CHECK_RESULT(uniformBuffers.scene.map());

	//// Vertex shader cube model uniform buffer blocks
	//for (auto& cube : cubes) {
	//	VK_CHECK_RESULT(vulkanDevice->createBuffer(
	//		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	//		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	//		&cube.uniformBuffer,
	//		sizeof(glm::mat4)));
	//	VK_CHECK_RESULT(cube.uniformBuffer.map());
	//}

	updateUniformBuffers();
	//updateCubeUniformBuffers();
}

void Texture::updateUniformBuffers()
{

}

void Texture::prepare(const std::string &textpath, VkSampleCountFlagBits _sampleCount)
{
	 path = textpath;

	/*
	Extension specific functions
	*/

	// The push descriptor update function is part of an extension so it has to be manually loaded 
	vkCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(device, "vkCmdPushDescriptorSetKHR");
	if (!vkCmdPushDescriptorSetKHR) {
		vks::tools::exitFatal("Could not get a valid function pointer for vkCmdPushDescriptorSetKHR", -1);
	}

		//loadTexture(path);
		//generateMark();
		setupVertexDescriptions();
		prepareUniformBuffers();
		setupDescriptorSetLayout();
		preparePipelines(path, _sampleCount);
		setupDescriptorPool();
		//setupDescriptorSet();
		//buildCommandBuffers(cmdBuffers);

}