#include "renderer/uiRender.h"
#pragma once
#include "utils/loadshader.h"


UiRender::UiRender()
{
}

UiRender::~UiRender()
{
}
void UiRender::createUi(vks::VulkanDevice *vulkanDevice, VkQueue queue, std::vector<VkFramebuffer> &frameBuffers, UiInfo uiInfo) {


		vks::UIOverlayCreateInfo overlayCreateInfo = {};
		// Setup default overlay creation info
		overlayCreateInfo.device = vulkanDevice;
		overlayCreateInfo.copyQueue = queue;
		overlayCreateInfo.framebuffers = frameBuffers;
		overlayCreateInfo.colorformat = uiInfo.colorFormat;
		overlayCreateInfo.depthformat = uiInfo.depthFormat;
		overlayCreateInfo.width = uiInfo.width;
		overlayCreateInfo.height = uiInfo.height;
		// Virtual function call for example to customize overlay creation
	//	OnSetupUIOverlay(overlayCreateInfo);
		// Load default shaders if not specified by example
		if (overlayCreateInfo.shaders.size() == 0) {
			overlayCreateInfo.shaders = {
				loadShader(getAssetPath + "/base/uioverlay.vert.spv", VK_SHADER_STAGE_VERTEX_BIT,vulkanDevice->logicalDevice,shaderModules),
				loadShader(getAssetPath + "/base/uioverlay.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT,vulkanDevice->logicalDevice,shaderModules),
			};
		}
		UIOverlay = new vks::UIOverlay(overlayCreateInfo);
		
}
void UiRender::updateOverlay(UiInfo & uiInfo, UiComponents &uiComponents,std::function<int (vks::UIOverlay *overlay)> lambdaCallBabck)
{
	       
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)uiInfo.width, (float)uiInfo.height);
		io.DeltaTime = uiComponents.frameTimer;
		io.MousePos = ImVec2(uiComponents.mousePos.x, uiComponents.mousePos.y);
		io.MouseDown[0] = uiComponents.mouse.left;
		io.MouseDown[1] = uiComponents.mouse.right;
		ImGui::NewFrame();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::SetNextWindowPos(ImVec2(10, 10));
		ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Vulkan", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		ImGui::TextUnformatted(uiComponents.title.c_str());
		ImGui::TextUnformatted(uiComponents.deviceName.c_str());
		ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / uiComponents.lastFPS), uiComponents.lastFPS);


		ImGui::PushItemWidth(110.0f * UIOverlay->scale);
		int a =lambdaCallBabck(UIOverlay);
		ImGui::PopItemWidth();


		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::Render();

		UIOverlay->update();
}
void UiRender::OnSetupUIOverlay(vks::UIOverlayCreateInfo overlayCreateInfo)
{

}
//void UiRender::OnUpdateUIOverlay(vks::UIOverlay *overlay)
//{
//
//		
//}