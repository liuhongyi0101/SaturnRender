#pragma once
#include "vulkan/vulkan.h"
#include "VulkanInitializers.hpp"
#include "VulkanModel.hpp"
#define VERTEX_BUFFER_BIND_ID 0
class VertexDescriptions
{
public:
	VertexDescriptions();
	~VertexDescriptions();

	void setupVertexDescriptions();
	struct vertexInputState {
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	};

	vertexInputState vertices;

	vks::VertexLayout vertexLayout = vks::VertexLayout({
	vks::VERTEX_COMPONENT_POSITION,
	vks::VERTEX_COMPONENT_NORMAL,
	vks::VERTEX_COMPONENT_UV
		});

};

