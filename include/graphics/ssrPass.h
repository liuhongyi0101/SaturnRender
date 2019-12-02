#pragma once
#include "graphics/basePass.h"
class SsrPass :public BasePass
{
public:
	SsrPass(vks::VulkanDevice * vulkanDevice);

	void createFramebuffersAndRenderPass(uint32_t width, uint32_t  height);
	void createPipeline();
	void createDescriptorsLayouts(VkDescriptorPool &descriptorPool);
	~SsrPass();


	struct SsrRtFrameBuffer : public FrameBuffer {
		FrameBufferAttachment ssrRtAttachment;
	} ssrRtFrameBuffer;


private:

};