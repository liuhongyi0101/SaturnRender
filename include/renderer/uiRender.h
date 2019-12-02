#pragma once
#include "VulkanUIOverlay.h"
#include <functional>

struct UiInfo
{
	uint32_t width, height;
	VkFormat colorFormat,depthFormat;
};
struct UiComponents {
	glm::vec2 mousePos;
	mouseButtons mouse;
	float frameTimer;
	uint32_t lastFPS;
	std::string title, deviceName;

};
class UiRender
{
public:
	UiRender();
	~UiRender();
	void createUi(vks::VulkanDevice *vulkanDevice, VkQueue queue, std::vector<VkFramebuffer> &frameBuffers, UiInfo uiInfo);
	void OnSetupUIOverlay(vks::UIOverlayCreateInfo overlayCreateInfo);
	/*void OnUpdateUIOverlay(vks::UIOverlay *overlay);*/
	void updateOverlay(UiInfo & uiInfo, UiComponents &uiComponents, std::function<int(vks::UIOverlay *overlay)> lambdaCallBabck);

	std::vector<VkShaderModule> shaderModules;
	vks::UIOverlay *UIOverlay = nullptr;
private:

};