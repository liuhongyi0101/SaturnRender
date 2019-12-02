#include "renderer/renderer.h"
#include "utils/loadshader.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

	Renderer::Renderer() : VulkanExampleBase(false) {

		title = "blade";
		paused = false;
		timerSpeed *= 0.25f;
		settings.overlay = true;
		material = std::make_shared<Materials>();
		material->setMateiral();
		control = std::make_shared<Control>();
		control->setupCamera(camera,width, height);
		objectNames = { "Venus" };
	}
	
	void Renderer::loadAssets(vks::VulkanDevice *vulkanDevice, VkQueue queue)
	{

		std::vector<std::string> filenames = {  "cerberus/cerberus.fbx" };
		models.skybox.loadFromFile(modelPath + "models/cube.obj", vdo->vertexLayout, 1.0f, vulkanDevice, queue);
		for (auto file : filenames) {
			vks::Model model;
			model.loadFromFile(modelPath + "models/" + file, vdo->vertexLayout, OBJ_DIM * (file == "cerberus/cerberus.fbx" ? 1.0f : 1.0f), vulkanDevice, queue);
			models.objects.push_back(model);
		}
		models.scenes.loadFromFile(modelPath + "models/samplescene.dae", vdo->vertexLayout, 0.25f, vulkanDevice, queue);
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
		
		// 3D object
		descriptorSets->uboMatrices.projection = camera.matrices.perspective;
		descriptorSets->uboMatrices.view = camera.matrices.view;
		descriptorSets->uboMatrices.model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f + (models.objectIndex == 1 ? 45.0f : 0.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
		descriptorSets->uboMatrices.camPos = camera.position * -1.0f;
		glm::vec4 lightpos = descriptorSets->uboParams.lights[0];
		lightpos = camera.matrices.view * descriptorSets->uboMatrices.model * lightpos;
		descriptorSets->uboParams.lights[0] = lightpos;
		descriptorSets->uboMatrices.depthBiasMVP = descriptorSets->uboShadowMapMvpVS.depthMVP;
		memcpy(descriptorSets->uniformBuffers.object.mapped, &descriptorSets->uboMatrices, sizeof(descriptorSets->uboMatrices));

		// Skybox
		
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
	
	void Renderer::setupDepthStencil()
	{
	
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
	
	void Renderer::setupRenderPass()
	{
		std::array<VkAttachmentDescription, 2> attachments = {};
		// Color attachment
		attachments[0].format = swapChain.colorFormat;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		// Depth attachment
		attachments[1].format = depthFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 1;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;
		subpassDescription.pDepthStencilAttachment = &depthReference;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = nullptr;

		// Subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
	}
	void Renderer::buildCommandBuffers(VkRenderPass renderPass, std::vector<VkCommandBuffer>& drawCmdBuffers, std::vector<VkFramebuffer> &frameBuffers)
	{
		renderpassunit.renderPass = shadowMapPass->renderPass;
		renderpassunit.frameBuffer = shadowMapPass->frameBuffer;
		renderpassunit.descriptorSet = descriptorSets->descriptorSetShadowMap;
		renderpassunit.height = shadowMapPass->height;
		renderpassunit.width = shadowMapPass->width;
		renderpassunit.commandBuffer = shadowMapPass->commandBuffer;
		renderpassunit.models = models;
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

		// deferred rendering
		submitInfo.pWaitSemaphores = &shadowMapPass->semaphore;
		submitInfo.pSignalSemaphores = &deferredPass->semaphore;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &deferredPass->cmdBuffer;
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

		// SSAO rendering
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
			if (overlay->comboBox("Object type", &models.objectIndex, objectNames)) {
				updateUniformBuffers();
				buildCommandBuffers(renderPass, drawCmdBuffers, frameBuffers);
			}
		}
		return 0;
	}
	Renderer:: ~Renderer() {

		vkDestroyPipeline(device, pipeline->pipelines["opaquePipeline"], nullptr);

		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSets->descriptorSetLayout, nullptr);

		for (auto& model : models.objects) {
			model.destroy();
		}
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
		ssaoPass->models.scene = models.scenes;
		ssaoPass->preparePipelines(vdo,renderPass);
	
		pipeline->createQuadPipeline(device, renderPass, ssaoPass->pipelineLayouts.composition, "outputPipeline");
		ssaoPass->buildDeferredCommandBuffer(pipeline);
	}
	void Renderer::loadModule() {
		
		vdo = std::make_shared<VertexDescriptions>();
		vdo->setupVertexDescriptions();
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

		irradianceCube = std::make_shared<IrradianceCube>(vulkanDevice, cmdPool,  vdo,models, queue);
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
		deferredPass->buildCommandBuffer(cmdPool,models.scenes.vertices.buffer, models.scenes.indices.buffer, models.scenes.indexCount);

		skyboxPass  = std::make_shared<SkyboxPass>(vulkanDevice);
		skyboxPass->createModel(queue, vdo->vertexLayout);
		skyboxPass->createDescriptorsLayouts();
		skyboxPass->createUniformBuffers(queue, camera.matrices.perspective, glm::mat4(glm::mat3(camera.matrices.view)));
		skyboxPass->wirteDescriptorSets(descriptorSets->descriptorPool, descriptorSets->cubeMap.descriptor);
		skyboxPass->createPipeline(vdo->vertices.inputState,renderPass);
		
		/*
		msaa_ = std::make_shared<Msaa>(vulkanDevice);
		sampleCount = msaa_->getMaxUsableSampleCount();
		msaa_->setupRenderPass(&offscreenPass.renderPass);*/
	}

