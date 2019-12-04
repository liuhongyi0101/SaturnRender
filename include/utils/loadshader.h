#pragma once
#include "VulkanTools.h"
#ifndef NAME_H
#define NAME_H

VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage, VkDevice device, std::vector<VkShaderModule> &shaderModules);
const std::string getAssetPath  =  "../../shader/";
const std::string modelPath = "../../assets/";
	
#endif