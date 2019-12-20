#pragma once
#include "renderer/sceneGraph.h"
#include "bobject/material.h"
#include "renderer/descriptor.h"
#include "renderer/vertexDescriptions.h"
#include "renderer/pipeline.h"
#include "renderer/texture.h"
#include "vulkanexamplebase.h"
#include "renderer/command.h"
#include "renderer/control.h"
#include "renderer/uiRender.h"
#include "renderer/IrradianceCube.h"
#include "renderer/shadowMapPass.h"
#include "renderer/ssaoPass.h"
#include "graphics/gbufferPass.h"
#include "graphics/skyboxPass.h"
#include "graphics/ssrPass.h"
#include "graphics/deferedShading.h"
#include "graphics/prezPass.h"
#include "graphics/bloomFFT.h"
#include "graphics/ouputPass.h"
#define FB_COLOR_FORMAT VK_FORMAT_R8G8B8A8_UNORM
#define GRID_DIM 2
#define OBJ_DIM 0.1f
#define VERTEX_BUFFER_BIND_ID 0

	// Renderer Engine
	class Renderer : public VulkanExampleBase
	{
#pragma region Field

	public:
	
		renderpassUnit renderpassunit;
		VkPipelineLayout pipelineLayout;
		UiInfo uiinfo;
		UiComponents uiComponents;

	private:
		std::shared_ptr<SceneGraph> sceneGraph;
		std::shared_ptr<Pipeline> pipeline;
		std::shared_ptr<VertexDescriptions> vdo;
		std::shared_ptr<Descriptor> descriptorSets;
		std::shared_ptr<Materials> material;
		std::shared_ptr<Command> command;
		std::shared_ptr<Control> control;
		std::shared_ptr<UiRender> uiRender;
		std::shared_ptr<IrradianceCube> irradianceCube;
		std::shared_ptr<ShadowMapPass> shadowMapPass;
		std::shared_ptr<SsaoPass> ssaoPass;
		std::shared_ptr<GbufferPass> gbufferPass;
		std::shared_ptr<SkyboxPass>  skyboxPass;
		std::shared_ptr<SsrPass>  ssrPass;
		std::shared_ptr<DeferredShading> deferredShading;
		std::shared_ptr<PrezPass> prezPass;
		std::shared_ptr<BloomFFT> bloomFFT;
		std::shared_ptr<OutputPass> outputPass;
	protected:
		
		vks::Buffer uniformBuffer;
	
		glm::vec3 lightPos = glm::vec3();
		float lightFOV = 60.0f;
	// Keep depth range as small as possible
	// for better shadow map precision
		float zNear = 1.0f;
		float zFar = 100.0f;

#pragma endregion

#pragma region function	
	public:
		Renderer();
		~Renderer();
	
		virtual void render();
		void loadModule();
		virtual void viewChanged();
		int OnUpdateUIOverlay(vks::UIOverlay *overlay);

		void loadAssets(vks::VulkanDevice *vulkanDevice, VkQueue queue);
		void prepareUniformBuffers(vks::VulkanDevice *vulkanDevice);
		void updateUniformBuffers();
		void updateLights();
		void updateUniformBufferShadowMap();
	
	
		void buildCommandBuffers();
		void prepare();
		void PostProcessing();
		void draw();

	
	private:
		//void destroyResource();
	
#pragma endregion
	};

