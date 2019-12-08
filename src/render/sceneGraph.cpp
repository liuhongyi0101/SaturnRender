#include"renderer/sceneGraph.h"
#include "utils/loadshader.h"
SceneGraph::SceneGraph()
{
}

SceneGraph::~SceneGraph()
{
}

void SceneGraph::loadAssets(vks::VulkanDevice *vulkanDevice, VkQueue queue, std::shared_ptr<VertexDescriptions> vdo)
{

	models.skybox.loadFromFile(modelPath + "models/cube.obj", vdo->vertexLayout, 1.0f, vulkanDevice, queue);
	for (auto file : filenames) {
		vks::Model model;
		model.loadFromFile(modelPath + "models/" + file, vdo->vertexLayout, 0.25, vulkanDevice, queue);
		models.nodes[file] = model;
	}
}
void SceneGraph::createScene(vks::VulkanDevice *vulkanDevice, VkQueue queue, std::shared_ptr<VertexDescriptions> vdo)
{
	loadAssets(vulkanDevice, queue, vdo);
}