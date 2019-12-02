#pragma once
#include <vector>
#include "VulkanModel.hpp"

struct Meshes {
	std::vector<vks::Model> objects;
	vks::Model scenes;
	
	vks::Model skybox;

	int32_t objectIndex = 0;
};