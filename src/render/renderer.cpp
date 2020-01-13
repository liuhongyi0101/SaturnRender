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
		descriptorSets->colorMap.loadFromFile(modelPath + "textures/vulkan_cloth_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);
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
		glm::vec4 re = depthViewMatrix * glm::vec4(1.0,10.0,1.0, 1.0);
		memcpy(descriptorSets->uniformBuffers.shadowMapMvp.mapped, &descriptorSets->uboShadowMapMvpVS, sizeof(descriptorSets->uboShadowMapMvpVS));
	}
	
	
	void Renderer::buildCommandBuffers()
	{
		renderpassunit.renderPass = shadowMapPass->renderPass;
		renderpassunit.frameBuffer = shadowMapPass->frameBuffer;
		renderpassunit.descriptorSet = descriptorSets->descriptorSetShadowMap;
		renderpassunit.height = shadowMapPass->height;
		renderpassunit.width = shadowMapPass->width;
		renderpassunit.commandBuffer = shadowMapPass->commandBuffer;
		renderpassunit.models = sceneGraph->models.nodes[MODEL];
		renderpassunit.pipelineFact = pipeline;
		renderpassunit.pipelineLayout = pipelineLayout;

		command->buildShadowMapCommandBuffer(renderpassunit);

		//renderpassunit.mat = material->materials[material->materialIndex];
	}
	void Renderer::prepare()
	{

		gbufferPass = std::make_shared<GbufferPass>(vulkanDevice);
		gbufferPass->createFramebuffersAndRenderPass(width, height);
		VulkanExampleBase::prepare();
		loadModule();
		prepareUniformBuffers(vulkanDevice);
		
		pipeline->createShadowPipeline(device, shadowMapPass->renderPass, pipelineLayout, vdo);
		buildCommandBuffers();
		gbufferPass->buildCommandBuffer(cmdPool, sceneGraph->models.nodes[MODEL].vertices.buffer, sceneGraph->models.nodes[MODEL].indices.buffer, sceneGraph->models.nodes[MODEL].indexCount);
		PostProcessing();
		
		
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

		// Gbuffer gen
		submitInfo.pWaitSemaphores = &shadowMapPass->semaphore;
		submitInfo.pSignalSemaphores = &gbufferPass->semaphore;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &gbufferPass->cmdBuffer;
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

		// postprocess rendering
		submitInfo.pWaitSemaphores = &gbufferPass->semaphore;
		submitInfo.pSignalSemaphores = &ssaoPass->semaphore;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &graphicCommand->cmdBuffer;
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

		// Scene rendering
		submitInfo.pWaitSemaphores = &ssaoPass->semaphore;
		submitInfo.pSignalSemaphores = &semaphores.renderComplete;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &outputPass->drawCmdBuffers[currentBuffer];
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

		VulkanExampleBase::submitFrame();

		vkWaitForFences(device, 1, &bloomFFT->compute.fence, VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &bloomFFT->compute.fence);

		VkSubmitInfo computeSubmitInfo = vks::initializers::submitInfo();
	
		computeSubmitInfo.commandBufferCount = 1;
		computeSubmitInfo.pCommandBuffers = &bloomFFT->compute.commandBuffer;

		VK_CHECK_RESULT(vkQueueSubmit(bloomFFT->compute.queue, 1, &computeSubmitInfo, bloomFFT->compute.fence));

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
		//	ssaoPass->updateUniformBufferSSAOParams(camera.matrices.perspective);
			gbufferPass->updateUniformBufferMatrices(camera.matrices.perspective, camera.matrices.view, descriptorSets->uboShadowMapMvpVS.depthMVP);
			glm::vec4 light = camera.matrices.view * descriptorSets->uboParams.lights[0];
			deferredShading->updateUniformBufferMatrices(light);
		}
		
		rayTracingPass->updateUniformBufferMatrices();
		rayTracingPass->wirteDescriptorSets();
		graphicCommand->startRecordCmd();
		rayTracingPass->buildCommandBuffer(graphicCommand->cmdBuffer);
		graphicCommand->stopRecordCmd();
		rayTracingPass->ping = 1 - rayTracingPass->ping;
		
		outputPass->updateUniformBufferMatrices();
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
			//	buildCommandBuffers(renderPass, drawCmdBuffers, frameBuffers);
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
		ssaoPass = std::make_shared<SsaoPass>(vulkanDevice,width,height);
		ssaoPass->prepareUniformBuffers(queue, camera.matrices.perspective);
		ssaoPass->setupLayoutsAndDescriptors(gbufferPass->deferredFrameBuffers.position.view, gbufferPass->deferredFrameBuffers.normal.view, gbufferPass->deferredFrameBuffers.albedo.view);
		
		ssaoPass->preparePipelines(vdo);
	
	
		ssrPass = std::make_shared<SsrPass>(vulkanDevice); 
		ssrPass->createFramebuffersAndRenderPass(width, height);
		ssrPass->createDescriptorsLayouts();
		ssrPass->createPipeline();
		ssrPass->createUniformBuffers(queue, camera.matrices.perspective);

		deferredShading = std::make_shared<DeferredShading>(vulkanDevice);
		deferredShading->createRenderPass(width, height);
		deferredShading->createFrameBuffer(gbufferPass->deferredFrameBuffers.depth.view);
		deferredShading->createDescriptorsLayouts();
		deferredShading->createPipeline();
		glm::vec4 light= camera.matrices.view * descriptorSets->uboParams.lights[0];
		deferredShading->createUniformBuffers(queue, light);


		std::vector<VkDescriptorImageInfo>	imageDescriptors = {
		vks::initializers::descriptorImageInfo(ssrPass->colorSampler, gbufferPass->deferredFrameBuffers.position.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		vks::initializers::descriptorImageInfo(ssrPass->colorSampler, gbufferPass->deferredFrameBuffers.normal.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		vks::initializers::descriptorImageInfo(ssrPass->colorSampler, gbufferPass->deferredFrameBuffers.albedo.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		vks::initializers::descriptorImageInfo(ssrPass->colorSampler, gbufferPass->deferredFrameBuffers.mix.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		vks::initializers::descriptorImageInfo(ssaoPass->colorSampler, ssaoPass->frameBuffers.ssaoBlur.color.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		vks::initializers::descriptorImageInfo(ssaoPass->colorSampler, deferredShading->deferredShadingRtFrameBuffer.deferredShadingRtAttachment.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		vks::initializers::descriptorImageInfo(prezPass->colorSampler, prezPass->prezRtFrameBuffer.prezRtAttachment.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		};
		
		std::vector<VkDescriptorImageInfo>	IblImageDescriptors = {
		irradianceCube->textures.irradianceCube.descriptor,
		irradianceCube->textures.prefilteredCube.descriptor,
		irradianceCube->textures.lutBrdf.descriptor
		};

		deferredShading->wirteDescriptorSets(descriptorSets->descriptorPool, imageDescriptors, IblImageDescriptors);
		ssrPass->wirteDescriptorSets(descriptorSets->descriptorPool, imageDescriptors);
		bloomFFT->prepareCompute(descriptorSets->descriptorPool, imageDescriptors[4]);
		
		ssaoPass->buildCommandBuffer(graphicCommand->cmdBuffer);

		prezPass->buildCommandBuffer(graphicCommand->cmdBuffer, sceneGraph->models.nodes[MODEL].vertices.buffer, sceneGraph->models.nodes[MODEL].indices.buffer, sceneGraph->models.nodes[MODEL].indexCount);
		deferredShading->buildCommandBuffer(graphicCommand->cmdBuffer);
		skyboxPass->createPipeline(vdo->vertices.inputState, deferredShading->renderPass);
		skyboxPass->buildCommandBuffer(graphicCommand->cmdBuffer);
		vkCmdEndRenderPass(graphicCommand->cmdBuffer);
		bloomFFT->buildComputeCommandBuffer();
	
		// ssr
	//	ssrPass->buildCommandBuffer(graphicCommand->cmdBuffer);
		rayTracingPass->buildCommandBuffer(graphicCommand->cmdBuffer);
		graphicCommand->stopRecordCmd();
		imageDescriptors = {
		vks::initializers::descriptorImageInfo(deferredShading->colorSampler, deferredShading->deferredShadingRtFrameBuffer.deferredShadingRtAttachment.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		};
		outputPass->wirteDescriptorSets(descriptorSets->descriptorPool, rayTracingPass->pingpongDescriptor[rayTracingPass->ping]);
		outputPass->buildCommandBuffer(graphicCommand->cmdBuffer, width, height);
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


		outputPass = std::make_shared<OutputPass>(vulkanDevice);
		outputPass->createRenderPass(depthFormat, swapChain.colorFormat);
		outputPass->createFrameBuffer(swapChain, width, height,cmdPool);
		outputPass->createDescriptorsLayouts();
		outputPass->createPipeline();
		outputPass->createUniformBuffers(queue);


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
		uiRender->createUi(vulkanDevice, queue, outputPass->frameBuffers, uiinfo);
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
		
	
		gbufferPass->createUniformBuffers(queue, camera.matrices.perspective, camera.matrices.view, descriptorSets->uboShadowMapMvpVS.depthMVP);
		gbufferPass->createDescriptorsLayouts(descriptorSets->descriptorPool);

		std::vector<VkDescriptorImageInfo> texDescriptor = {
			descriptorSets->shadowMapTexDescriptor,
			descriptorSets->pbrTex.albedoMap.descriptor,
			descriptorSets->pbrTex.normalMap.descriptor,
			descriptorSets->pbrTex.roughnessMap.descriptor,
			descriptorSets->pbrTex.metallicMap.descriptor,
			descriptorSets->pbrTex.aoMap.descriptor

		};
		gbufferPass->wirteDescriptorSets(descriptorSets->descriptorPool, texDescriptor);
		gbufferPass->createPipeline(vdo->vertices.inputState);
		
		

		skyboxPass  = std::make_shared<SkyboxPass>(vulkanDevice);
		skyboxPass->createModel(queue, vdo->vertexLayout);
		skyboxPass->createDescriptorsLayouts();
		skyboxPass->createUniformBuffers(queue, camera.matrices.perspective, glm::mat4(glm::mat3(camera.matrices.view)));
		skyboxPass->wirteDescriptorSets(descriptorSets->descriptorPool, descriptorSets->cubeMap.descriptor);
		


		prezPass = std::make_shared<PrezPass>(vulkanDevice);
		prezPass->createRenderPass(width,height);
		prezPass->createFrameBuffer();
		prezPass->createDescriptorsLayouts();
		prezPass->createPipeline(vdo->vertices.inputState);
		glm::mat4 mvp = camera.matrices.perspective * camera.matrices.view;
		prezPass->createUniformBuffers(queue, mvp);
		prezPass->wirteDescriptorSets(descriptorSets->descriptorPool);

		bloomFFT = std::make_shared<BloomFFT>(vulkanDevice);
		bloomFFT->prepareTextureTarget(width,height,cmdPool,queue);
		

		graphicCommand = std::make_shared<GraphicCommand>(vulkanDevice);
		rayTracingPass = std::make_shared<RayTracingPass>(vulkanDevice);

		rayTracingPass->createNoiseTex(queue);
		rayTracingPass->createFramebuffersAndRenderPass(width,height);
		rayTracingPass->createDescriptorsLayouts(descriptorSets->descriptorPool);
		rayTracingPass->createPipeline();
		rayTracingPass->createUniformBuffers(queue, camera.matrices.perspective *camera.matrices.view, camera.position);
		rayTracingPass->wirteDescriptorSets();
	}

