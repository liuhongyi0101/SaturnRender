#pragma once
#include <vector>
#include<map>
#include "VulkanModel.hpp"
#include "renderer/vertexDescriptions.h"
#define MODEL "cerberus/cerberus.fbx"
struct Models {
	std::map<std::string, vks::Model> nodes;
	vks::Model skybox;
	int32_t objectIndex = 0;
};
class SceneGraph
{
public:
	SceneGraph();
	~SceneGraph();
	 void loadAssets(vks::VulkanDevice *vulkanDevice, VkQueue queue, std::shared_ptr<VertexDescriptions> vdo);
     void createScene(vks::VulkanDevice *vulkanDevice, VkQueue queue, std::shared_ptr<VertexDescriptions> vdo);
	 Models	models;
	 std::vector<std::string> filenames = { "cerberus/cerberus.fbx",MODEL };


};