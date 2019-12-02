#include "renderer/vertexDescriptions.h"

VertexDescriptions::VertexDescriptions()
{
}

VertexDescriptions::~VertexDescriptions()
{
}
void VertexDescriptions::setupVertexDescriptions()
{

	// Binding description
	vertices.bindingDescriptions.resize(1);

	vertices.bindingDescriptions[0] =
		vks::initializers::vertexInputBindingDescription(
			VERTEX_BUFFER_BIND_ID,
			vertexLayout.stride(),
			VK_VERTEX_INPUT_RATE_VERTEX);



	// Attribute descriptions
	vertices.attributeDescriptions.resize(3);
	
	// Location 0 : Position
	vertices.attributeDescriptions[0] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			0);

	// Location 2 : Normal
	vertices.attributeDescriptions[1] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			1,
			VK_FORMAT_R32G32B32_SFLOAT,
			sizeof(float) * 3);

	vertices.attributeDescriptions[2] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			2,
			VK_FORMAT_R32G32_SFLOAT,
			sizeof(float) * 6);
	

	
	vertices.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
	vertices.inputState.vertexBindingDescriptionCount = vertices.bindingDescriptions.size();
	vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
	vertices.inputState.vertexAttributeDescriptionCount = vertices.attributeDescriptions.size();
	vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();

}