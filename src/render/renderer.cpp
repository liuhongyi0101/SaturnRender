#include "renderer/renderer.h"
#include "utils/loadshader.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

	Renderer::Renderer() : VulkanExampleBase(true) {

		title = "blade";
		paused = false;
		timerSpeed *= 0.25f;
		settings.overlay = true;
		material = std::make_shared<Materials>();
		material->setMateiral();
		control = std::make_shared<Control>();
		control->setupCamera(camera,width, height);
		
	}
	
	void Renderer::loadAssets(vks::VulkanDevice *vulkanDevice, VkQueue queue)
	{

		descriptorSets->cubeMap.loadFromFile(modelPath + "textures/hdr/pisa_cube.ktx", VK_FORMAT_R16G16B16A16_SFLOAT, vulkanDevice, queue);
		descriptorSets->colorMap.loadFromFile(modelPath + "textures/vulkan_cloth_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue);
		descriptorSets->pbrTex.albedoMap.loadFromFile(modelPath + "models/cerberus/albedo.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue);
		descriptorSets->pbrTex.normalMap.loadFromFile(modelPath + "models/cerberus/normal.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue);
		descriptorSets->pbrTex.aoMap.loadFromFile(modelPath + "models/cerberus/ao.ktx", VK_FORMAT_R8_UNORM, vulkanDevice, queue);
		descriptorSets->pbrTex.metallicMap.loadFromFile(modelPath + "models/cerberus/metallic.ktx", VK_FORMAT_R8_UNORM, vulkanDevice, queue);
		descriptorSets->pbrTex.roughnessMap.loadFromFile(modelPath + "models/cerberus/roughness.ktx", VK_FORMAT_R8_UNORM, vulkanDevice, queue);
	}
		// Prepare and initialize uniform buffer containing shader uniforms
	void Renderer::prepareUniformBuffers(vks::VulkanDevice *vulkanDevice)
	{
		updateLights();
		updateUniformBufferShadowMap();
		updateUniformBuffers();
	}
	void Renderer::updateUniformBuffers()
	{

		glm::vec4 lightpos = descriptorSets->uboParams.lights[0];
		lightpos = camera.matrices.view * descriptorSets->uboMatrices.model * lightpos;
		descriptorSets->uboParams.lights[0] = lightpos;
		descriptorSets->uboMatrices.depthBiasMVP = descriptorSets->uboShadowMapMvpVS.depthMVP;
		memcpy(descriptorSets->uniformBuffers.object.mapped, &descriptorSets->uboMatrices, sizeof(descriptorSets->uboMatrices));
	
		descriptorSets->uboVS.projection = camera.matrices.perspective;

		descriptorSets->uboVS.model = glm::mat4(glm::mat3(camera.matrices.view));
		memcpy(descriptorSets->uniformBuffers.skybox.mapped, &descriptorSets->uboVS, sizeof(descriptorSets->uboVS));
	}

	void Renderer::updateLights()
	{
		const float p = 15.0f;
		
		descriptorSets->uboParams.lights[1] = glm::vec4(-p, -p * 0.5f, p, 1.0f);
		descriptorSets->uboParams.lights[2] = glm::vec4(p, -p * 0.5f, p, 1.0f);
		descriptorSets->uboParams.lights[3] = glm::vec4(p, -p * 0.5f, -p, 1.0f);

		descriptorSets->uboParams.lights[0].x = cos(glm::radians( 360.0f)) * 40.0f;
		descriptorSets->uboParams.lights[0].y = -50.0f + sin(glm::radians( 360.0f)) * 20.0f;
		descriptorSets->uboParams.lights[0].z = 25.0f + sin(glm::radians( 360.0f)) * 5.0f;
		if (!paused)
		{
			//descriptorSets->uboParams.lights[0].x = sin(glm::radians(timer * 360.0f)) * 20.0f;
			//descriptorSets->uboParams.lights[0].z = cos(glm::radians(timer * 360.0f)) * 20.0f;
			/*descriptorSets->uboParams.lights[1].x = cos(glm::radians(timer * 360.0f)) * 20.0f;
			descriptorSets->uboParams.lights[1].y = sin(glm::radians(timer * 360.0f)) * 20.0f;*/
		}
		memcpy(descriptorSets->uniformBuffers.params.mapped, &descriptorSets->uboParams, sizeof(descriptorSets->uboParams));
	}
	void Renderer::updateUniformBufferShadowMap()
	{
		lightPos = descriptorSets->uboParams.lights[0];
		// Matrix from light's point of view
		glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(lightFOV), 1.0f, zNear, zFar);
		glm::mat4 depthViewMatrix = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));
		glm::mat4 depthModelMatrix = glm::mat4(1.0f);

		descriptorSets->uboShadowMapMvpVS.depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

		memcpy(descriptorSets->uniformBuffers.shadowMapMvp.mapped, &descriptorSets->uboShadowMapMvpVS, sizeof(descriptorSets->uboShadowMapMvpVS));
	}
	
	void Renderer::setupFrameBuffer()
	{
		VkImageView attachments[2];

		// Depth/Stencil attachment is the same for all frame buffers
		
		attachments[1] = deferredPass->deferredFrameBuffers.depth.view;

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.pNext = NULL;
		frameBufferCreateInfo.renderPass = renderPass;
		frameBufferCreateInfo.attachmentCount = 2;
		frameBufferCreateInfo.pAttachments = attachments;
		frameBufferCreateInfo.width = width;
		frameBufferCreateInfo.height = height;
		frameBufferCreateInfo.layers = 1;

		// Create frame buffers for every swap chain image
		frameBuffers.resize(swapChain.imageCount);
		for (uint32_t i = 0; i < frameBuffers.size(); i++)
		{
			attachments[0] = swapChain.buffers[i].view;
			VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &frameBuffers[i]));
		}
	}
	
	void Renderer::buildCommandBuffers(VkRenderPass renderPass, std::vector<VkCommandBuffer>& drawCmdBuffers, std::vector<VkFramebuffer> &frameBuffers)
	{
		renderpassunit.renderPass = shadowMapPass->renderPass;
		renderpassunit.frameBuffer = shadowMapPass->frameBuffer;
		renderpassunit.descriptorSet = descriptorSets->descriptorSetShadowMap;
		renderpassunit.height = shadowMapPass->height;
		renderpassunit.width = shadowMapPass->width;
		renderpassunit.commandBuffer = shadowMapPass->commandBuffer;
		renderpassunit.models = sceneGraph->models.nodes["samplescene.dae"];
		renderpassunit.pipelineFact = pipeline;
		renderpassunit.pipelineLayout = pipelineLayout;

		command->buildShadowMapCommandBuffer(renderpassunit);

		renderpassunit.renderPass = renderPass;
		renderpassunit.descriptorSet = descriptorSets->descriptorSet;
		renderpassunit.descriptorSetSkybox = descriptorSets->descriptorSetSkybox;
		renderpassunit.height = height;
		renderpassunit.width = width;
		renderpassunit.mat = material->materials[material->materialIndex];
		//command->buildCommandBuffers(renderpassunit, drawCmdBuffers, frameBuffers);



		renderpassunit.descriptorSet = ssaoPass->descriptorSets.composition;
		renderpassunit.pipelineLayout = ssaoPass->pipelineLayouts.composition;

		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

		VkClearValue clearValues[1];
		clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		//clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = clearValues;

		for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
		{
			// Set target frame buffer
			renderPassBeginInfo.framebuffer = frameBuffers[i];

			VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

			vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = vks::initializers::viewport((float)renderpassunit.width, (float)renderpassunit.height, 0.0f, 1.0f);
			vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

			VkRect2D scissor = vks::initializers::rect2D(renderpassunit.width, renderpassunit.height, 0, 0);
			vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

			vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderpassunit.pipelineLayout, 0, 1, &renderpassunit.descriptorSet, 0, NULL);

			// Final composition pass
			vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderpassunit.pipelineFact->pipelines["outputPipeline"]);
			vkCmdDraw(drawCmdBuffers[i], 3, 1, 0, 0);
			skyboxPass->buildCommandBuffer(drawCmdBuffers[i]);
			
			vkCmdEndRenderPass(drawCmdBuffers[i]);

			VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
		}
		
	}
	void Renderer::prepare()
	{
		deferredPass = std::make_shared<DeferredPass>(vulkanDevice);
		deferredPass->createFramebuffersAndRenderPass(width, height);
		VulkanExampleBase::prepare();
		loadModule();
		prepareUniformBuffers(vulkanDevice);
		pipeline->setupPipeline(device,renderPass, pipelineLayout, vdo);
		pipeline->createShadowPipeline(device, shadowMapPass->renderPass, pipelineLayout);
		PostProcessing();
		
		buildCommandBuffers(renderPass, drawCmdBuffers, frameBuffers);
		prepared = true;
	}
	void Renderer::draw()
	{
		VulkanExampleBase::prepareFrame();
		// Wait for swap chain presentation to finish
		submitInfo.pWaitSemaphores = &semaphores.presentComplete;
		submitInfo.pSignalSemaphores = &shadowMapPass->semaphore;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &shadowMapPass->commandBuffer;
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

		// Gbuffer rendering
		submitInfo.pWaitSemaphores = &shadowMapPass->semaphore;
		submitInfo.pSignalSemaphores = &deferredPass->semaphore;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &deferredPass->cmdBuffer;
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

		// postprocess rendering
		submitInfo.pWaitSemaphores = &deferredPass->semaphore;
		submitInfo.pSignalSemaphores = &ssaoPass->semaphore;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &ssaoPass->cmdBuffer;
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

		// Scene rendering
		submitInfo.pWaitSemaphores = &ssaoPass->semaphore;
		submitInfo.pSignalSemaphores = &semaphores.renderComplete;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

		VulkanExampleBase::submitFrame();

	}
	 void Renderer::render()
	{
		if (!prepared)
			return;
		draw();
	
		if (!paused)
		{
		//	updateLights();
		//	updateUniformBufferShadowMap();
			updateUniformBuffers();
			skyboxPass->updateUniformBuffer(camera.matrices.perspective, glm::mat4(glm::mat3(camera.matrices.view)));
			ssaoPass->updateUniformBufferMatrices(camera.matrices.perspective,camera.matrices.view);
		//	ssaoPass->updateUniformBufferSSAOParams(camera.matrices.perspective,glm::vec4(camera.position,1.0), descriptorSets->uboParams.lights[0]);
			deferredPass->updateUniformBufferMatrices(camera.matrices.perspective, camera.matrices.view, descriptorSets->uboShadowMapMvpVS.depthMVP);
		}
			
		uiComponents.frameTimer = frameTimer;
		uiComponents.lastFPS = lastFPS;
		uiComponents.mousePos = mousePos;
		uiComponents.mouse = mouseButtons;
		uiRender->updateOverlay(uiinfo, uiComponents, [this](vks::UIOverlay *overlay)->int {
			return  OnUpdateUIOverlay(overlay);
		});
	}

	 void Renderer::viewChanged()
	{
		updateUniformBuffers();
	}

	 int Renderer::OnUpdateUIOverlay(vks::UIOverlay *overlay)
	{
		if (overlay->header("Settings")) {
			if (overlay->comboBox("Material", &material->materialIndex, material->materialNames)) {
				buildCommandBuffers(renderPass, drawCmdBuffers, frameBuffers);
			}
			/*if (overlay->comboBox("Object type", &models.objectIndex, objectNames)) {
				updateUniformBuffers();
				buildCommandBuffers(renderPass, drawCmdBuffers, frameBuffers);
			}*/
		}
		return 0;
	}
	Renderer:: ~Renderer() {

		vkDestroyPipeline(device, pipeline->pipelines["opaquePipeline"], nullptr);

		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSets->descriptorSetLayout, nullptr);
		descriptorSets->uniformBuffers.object.destroy();
		descriptorSets->uniformBuffers.params.destroy();
	}
	void Renderer::PostProcessing()
	{
		ssaoPass = std::make_shared<SsaoPass>(vulkanDevice,cmdPool,width,height);
		ssaoPass->prepareUniformBuffers(queue, camera.matrices.perspective,camera.matrices.view, glm::vec4(camera.position, 1.0), descriptorSets->uboParams.lights[0]);
		ssaoPass->setupLayoutsAndDescriptors(deferredPass->deferredFrameBuffers.position.view, deferredPass->deferredFrameBuffers.normal.view, deferredPass->deferredFrameBuffers.albedo.view);
		ssaoPass->compositionSet(irradianceCube->textures.irradianceCube, irradianceCube->textures.prefilteredCube, irradianceCube->textures.lutBrdf,
			deferredPass->deferredFrameBuffers.position.view, deferredPass->deferredFrameBuffers.normal.view, deferredPass->deferredFrameBuffers.albedo.view);
		
		ssaoPass->preparePipelines(vdo,renderPass);
	
		pipeline->createQuadPipeline(device, renderPass, ssaoPass->pipelineLayouts.composition, "outputPipeline");
		

		ssrPass = std::make_shared<SsrPass>(vulkanDevice); 
		ssrPass->createFramebuffersAndRenderPass(width, height);
		ssrPass->createDescriptorsLayouts();
		ssrPass->createPipeline();
		ssrPass->createUniformBuffers(queue, camera.matrices.perspective);

		deferredShading = std::make_shared<DeferredShading>(vulkanDevice);
		deferredShading->createRenderPass(width, height);
		deferredShading->createFrameBuffer();
		deferredShading->createDescriptorsLayouts();
		deferredShading->createPipeline();
		deferredShading->createUniformBuffers(queue, descriptorSets->uboParams.lights[0]);


		std::vector<VkDescriptorImageInfo>	imageDescriptors = {
		vks::initializers::descriptorImageInfo(ssrPass->colorSampler, deferredPass->deferredFrameBuffers.position.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		vks::initializers::descriptorImageInfo(ssrPass->colorSampler, deferredPass->deferredFrameBuffers.normal.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		vks::initializers::descriptorImageInfo(ssrPass->colorSampler, deferredPass->deferredFrameBuffers.albedo.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		vks::initializers::descriptorImageInfo(ssaoPass->colorSampler, ssaoPass->frameBuffers.ssaoBlur.color.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		vks::initializers::descriptorImageInfo(ssaoPass->colorSampler, deferredShading->deferredShadingRtFrameBuffer.deferredShadingRtAttachment.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		};
		

		
		std::vector<VkDescriptorImageInfo>	IblImageDescriptors = {
		irradianceCube->textures.irradianceCube.descriptor,
		irradianceCube->textures.prefilteredCube.descriptor,
		irradianceCube->textures.lutBrdf.descriptor
		};

		deferredShading->wirteDescriptorSets(descriptorSets->descriptorPool, imageDescriptors, IblImageDescriptors);
		ssrPass->wirteDescriptorSets(descriptorSets->descriptorPool, imageDescriptors);


		ssaoPass->buildDeferredCommandBuffer(pipeline);
		deferredShading->buildCommandBuffer(ssaoPass->cmdBuffer);
		// ssr
		ssrPass->buildCommandBuffer(ssaoPass->cmdBuffer);
		VK_CHECK_RESULT(vkEndCommandBuffer(ssaoPass->cmdBuffer));
	}
	void Renderer::loadModule() {
		
		vdo = std::make_shared<VertexDescriptions>();
		vdo->setupVertexDescriptions();

		sceneGraph = std::make_shared<SceneGraph>();
		sceneGraph->createScene(vulkanDevice, queue, vdo);
		pipeline = std::make_shared<Pipeline>(device);
		descriptorSets = std::make_shared<Descriptor>(vulkanDevice);

		loadAssets(vulkanDevice, queue);
		command = std::make_shared<Command>(device);
		uiRender = std::make_shared<UiRender>();

		
		uiinfo.colorFormat = swapChain.colorFormat;
		uiinfo.depthFormat = depthFormat;
		uiinfo.width = width;
		uiinfo.height = height;

	
		uiComponents.deviceName = deviceProperties.deviceName;
		uiComponents.frameTimer = frameTimer;
		uiComponents.lastFPS = lastFPS;
		uiComponents.title = title;
		uiComponents.mousePos = mousePos;
		uiComponents.mouse = mouseButtons;
		uiRender->createUi(vulkanDevice, queue, frameBuffers, uiinfo);
		uiRender->updateOverlay(uiinfo, uiComponents, [&](vks::UIOverlay *overlay)->int {
			return  this->OnUpdateUIOverlay(overlay);
		});
		UIOverlay = uiRender->UIOverlay;

		irradianceCube = std::make_shared<IrradianceCube>(vulkanDevice, cmdPool,  vdo,sceneGraph->models.skybox, queue);
		irradianceCube->generateIrradianceCube(descriptorSets->cubeMap);
		irradianceCube->generatePrefilteredCube(descriptorSets->cubeMap);
		irradianceCube->generateBRDFLUT(descriptorSets->cubeMap);

		shadowMapPass = std::make_shared<ShadowMapPass>(vulkanDevice, cmdPool);
		shadowMapPass->prepareShadowMapPassFramebuffer();
		
		descriptorSets->setupDescriptorSetLayout(pipelineLayout);
		descriptorSets->setupShadowMapSet();
		descriptorSets->createShadowMapTexDescriptor(shadowMapPass->depthSampler, shadowMapPass->depth.view);
		descriptorSets->updateDescriptorSet(irradianceCube->textures.irradianceCube,irradianceCube->textures.prefilteredCube, irradianceCube->textures.lutBrdf);
		
	
		deferredPass->createUniformBuffers(queue, camera.matrices.perspective, camera.matrices.view, descriptorSets->uboShadowMapMvpVS.depthMVP);
		deferredPass->createDescriptorsLayouts(descriptorSets->descriptorPool);
		deferredPass->wirteDescriptorSets(descriptorSets->descriptorPool, descriptorSets->shadowMapTexDescriptor);
		deferredPass->createPipeline(vdo->vertices.inputState);
		
		deferredPass->buildCommandBuffer(cmdPool, sceneGraph->models.nodes["samplescene.dae"].vertices.buffer, sceneGraph->models.nodes["samplescene.dae"].indices.buffer, sceneGraph->models.nodes["samplescene.dae"].indexCount);

		skyboxPass  = std::make_shared<SkyboxPass>(vulkanDevice);
		skyboxPass->createModel(queue, vdo->vertexLayout);
		skyboxPass->createDescriptorsLayouts();
		skyboxPass->createUniformBuffers(queue, camera.matrices.perspective, glm::mat4(glm::mat3(camera.matrices.view)));
		skyboxPass->wirteDescriptorSets(descriptorSets->descriptorPool, descriptorSets->cubeMap.descriptor);
		skyboxPass->createPipeline(vdo->vertices.inputState,renderPass);
		
	

	}

