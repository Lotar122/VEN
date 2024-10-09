#include "Engine.hpp"
#include "Engine.Structs.InData.hpp"

namespace nihil::graphics 
{
	PipelineInfo Engine::CreatePipelineConfiguration(std::vector<VertexAttribute> attributes, std::vector<VertexBindingInformation> bindingInfo, vk::ShaderModule* vertexShader, vk::ShaderModule* fragmentShader)
	{
		PipelineInfo out;

		//first is binding and second is location
		std::vector<std::pair<uint32_t, uint32_t>> bindingLocationComb(0);

		for (VertexAttribute& attrib : attributes)
		{
			bool exists = false;
			for (std::pair<uint32_t, uint32_t>& comb : bindingLocationComb)
			{
				if (std::make_pair(attrib.binding, attrib.location) == comb) { exists = true; break; }
			}
			if (!exists)
			{
				bindingLocationComb.push_back(std::make_pair(attrib.binding, attrib.location));
			}
		}

		for (std::pair<uint32_t, uint32_t>& comb : bindingLocationComb)
		{
			vk::VertexInputBindingDescription bindingDesc;
			vk::VertexInputAttributeDescription attribDesc;

			attribDesc.binding = comb.first;
			attribDesc.location = comb.second;
			vk::Format format = vk::Format::eUndefined;
			uint32_t offset = 0;
			for (VertexAttribute& attrib : attributes)
			{
				if (attrib.binding == comb.first && attrib.location == comb.second)
				{
					format = attrib.format;
					offset = attrib.offset;
				}
			}
			attribDesc.format = format;
			attribDesc.offset = offset;
	
			bindingDesc.binding = comb.first;
			bindingDesc.inputRate = bindingInfo[comb.first].inputRate;

			bool existsA = false;
			bool existsB = false;

			for (vk::VertexInputBindingDescription& desc : out.bindingDesc)
			{
				if (desc.binding == bindingDesc.binding) { existsB = true; break; }
			}

			if (!existsB)
			{
				out.bindingDesc.push_back(bindingDesc);
			}

			for (vk::VertexInputAttributeDescription& desc : out.attributeDesc)
			{
				if (desc.binding == attribDesc.binding && desc.location == attribDesc.location) { existsA = true; break; }
			}

			if (!existsA)
			{
				out.attributeDesc.push_back(attribDesc);
			}
		}

		std::vector<uint32_t> bindingStride(out.bindingDesc.size());

		for (vk::VertexInputAttributeDescription& desc : out.attributeDesc)
		{
			bindingStride[desc.binding] += GetFormatLenght(desc.format);
		}
		for (vk::VertexInputBindingDescription& bind : out.bindingDesc)
		{
			bind.stride = bindingStride[bind.binding];
		}

		for (vk::VertexInputAttributeDescription& desc : out.attributeDesc)
		{
			std::cout << "binding: " << desc.binding << " " << "location: " << desc.location << " " << "offset: " << desc.offset << std::endl;
		}

		out.fragmentShader = fragmentShader;
		out.vertexShader = vertexShader;

		//! finish
		return out;
	}

	PipelineBundle Engine::CreatePipeline(PipelineInfo pipelineInfoN, vk::RenderPass _renderPass)
	{
		vk::GraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.flags = vk::PipelineCreateFlags();
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		vk::PipelineLayout layout;
		vk::Pipeline pipeline;

		PipelineBundle out;

		/*
		* create bindings for a mat4 by making 4 vec4, make sure to add offsets
		*/

		if (this->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Creating a vertex input " << GREEN << "[###]" << RESET << std::endl;
		//Vertex Input

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
		vertexInputInfo.vertexBindingDescriptionCount = pipelineInfoN.bindingDesc.size();
		vertexInputInfo.pVertexBindingDescriptions = pipelineInfoN.bindingDesc.data();
		vertexInputInfo.vertexAttributeDescriptionCount = pipelineInfoN.attributeDesc.size();
		vertexInputInfo.pVertexAttributeDescriptions = pipelineInfoN.attributeDesc.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;

		if (this->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Creating a input assembly " << GREEN << "[###]" << RESET << std::endl;
		//Input Assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
		inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
		inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;

		//Vertex shader
		// in class declaration: vk::ShaderModule* vertexShader = NULL;
		vk::PipelineShaderStageCreateInfo vertexShaderInfo = {};
		vertexShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
		vertexShaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
		vertexShaderInfo.module = *pipelineInfoN.vertexShader;
		vertexShaderInfo.pName = "main";
		shaderStages.push_back(vertexShaderInfo);

		if (this->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Creating the viewport " << GREEN << "[###]" << RESET << std::endl;
		//Viewport
		renderer->viewport.x = 0.0f;
		renderer->viewport.y = 0.0f;
		renderer->viewport.width = renderer->swapchainBundle.extent.width;
		renderer->viewport.height = renderer->swapchainBundle.extent.height;
		renderer->viewport.minDepth = 0.0f;
		renderer->viewport.maxDepth = 1.0f;

		renderer->scissor.offset.x = 0.0f;
		renderer->scissor.offset.y = 0.0f;
		renderer->scissor.extent = renderer->swapchainBundle.extent;
		vk::PipelineViewportStateCreateInfo viewportInfo = {};
		viewportInfo.flags = vk::PipelineViewportStateCreateFlags();
		viewportInfo.viewportCount = 1;
		viewportInfo.pViewports = &renderer->viewport;
		viewportInfo.scissorCount = 1;
		viewportInfo.pScissors = &renderer->scissor;

		// Define dynamic states
		std::vector<vk::DynamicState> dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		// Create pipeline dynamic state info
		vk::PipelineDynamicStateCreateInfo dynamicStateInfo(
			vk::PipelineDynamicStateCreateFlags(),
			dynamicStates.size(),
			dynamicStates.data()
		);

		pipelineInfo.pViewportState = &viewportInfo;
		pipelineInfo.pDynamicState = &dynamicStateInfo;

		if (this->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Creating a rasterizer " << GREEN << "[###]" << RESET << std::endl;
		//Rasterization
		vk::PipelineRasterizationStateCreateInfo rasterizationInfo = {};
		rasterizationInfo.flags = vk::PipelineRasterizationStateCreateFlags();
		rasterizationInfo.depthClampEnable = VK_FALSE;
		rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationInfo.polygonMode = vk::PolygonMode::eFill;
		rasterizationInfo.lineWidth = 1.0f;
		rasterizationInfo.cullMode = vk::CullModeFlagBits::eNone;
		//make editable
		rasterizationInfo.frontFace = vk::FrontFace::eCounterClockwise;
		rasterizationInfo.depthBiasEnable = VK_FALSE;
		pipelineInfo.pRasterizationState = &rasterizationInfo;

		//Fragment shader
		//in class declaration: vk::ShaderModule* fragmentShader = NULL;
		vk::PipelineShaderStageCreateInfo fragmentShaderInfo = {};
		fragmentShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
		fragmentShaderInfo.stage = vk::ShaderStageFlagBits::eFragment;
		fragmentShaderInfo.module = *pipelineInfoN.fragmentShader;
		fragmentShaderInfo.pName = "main";
		shaderStages.push_back(fragmentShaderInfo);

		if (this->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Setting shader stages " << GREEN << "[###]" << RESET << std::endl;
		//Set shaders
		pipelineInfo.stageCount = shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();

		if (this->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Creating a multisampler " << GREEN << "[###]" << RESET << std::endl;
		//Multisampling
		vk::PipelineMultisampleStateCreateInfo multisampleInfo = {};
		multisampleInfo.flags = vk::PipelineMultisampleStateCreateFlags();
		multisampleInfo.sampleShadingEnable = VK_FALSE;
		multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
		pipelineInfo.pMultisampleState = &multisampleInfo;

		if (this->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Specifying a color blender " << GREEN << "[###]" << RESET << std::endl;
		//Color blend
		vk::PipelineColorBlendAttachmentState attachment = {};
		attachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		attachment.blendEnable = VK_FALSE;
		vk::PipelineColorBlendStateCreateInfo colorBlendingInfo = {};
		colorBlendingInfo.flags = vk::PipelineColorBlendStateCreateFlags();
		colorBlendingInfo.logicOpEnable = VK_FALSE;
		colorBlendingInfo.logicOp = vk::LogicOp::eCopy;
		colorBlendingInfo.attachmentCount = 1;
		colorBlendingInfo.pAttachments = &attachment;
		colorBlendingInfo.blendConstants[0] = 0.0f;
		colorBlendingInfo.blendConstants[1] = 0.0f;
		colorBlendingInfo.blendConstants[2] = 0.0f;
		colorBlendingInfo.blendConstants[3] = 0.0f;
		pipelineInfo.pColorBlendState = &colorBlendingInfo;

		vk::PipelineDepthStencilStateCreateInfo depthInfo = {};
		depthInfo.depthTestEnable = VK_TRUE;
		depthInfo.depthWriteEnable = VK_TRUE;
		depthInfo.depthCompareOp = vk::CompareOp::eLess;

		pipelineInfo.pDepthStencilState = &depthInfo;

		if (this->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Creating a pipeline layout ";
		//Create pipeline layout (move to a seperate function in future)
		//in class declaration: vk::PipelineLayout layout;
		vk::PipelineLayoutCreateInfo layoutInfo = {};
		layoutInfo.flags = vk::PipelineLayoutCreateFlags();
		layoutInfo.setLayoutCount = 0;
		layoutInfo.pushConstantRangeCount = 1;

		vk::PushConstantRange pushConstantInfo = {};
		pushConstantInfo.offset = 0;
		pushConstantInfo.size = sizeof(ObjectData);
		pushConstantInfo.stageFlags = vk::ShaderStageFlagBits::eVertex;

		layoutInfo.pPushConstantRanges = &pushConstantInfo;

		try {
			layout = this->logicalDevice.createPipelineLayout(layoutInfo);
		}
		catch (vk::SystemError err) {
			std::cout << RED << "[###]" << RESET << std::endl;
			this->error = true;
			this->finishPipelineSetup();
			throw std::exception(err.what());
		}
		std::cout << GREEN << "[###]" << RESET << std::endl;

		

		//finish pipeline creation
		pipelineInfo.layout = layout;

		pipelineInfo.renderPass = _renderPass;
		pipelineInfo.basePipelineHandle = nullptr;

		try {
			pipeline = this->logicalDevice.createGraphicsPipeline(nullptr, pipelineInfo).value;
		}
		catch (vk::SystemError err) {
			this->error = true;
			this->finishPipelineSetup();
			throw std::exception(err.what());
		}
		this->finishPipelineSetup();

		out.renderPass = _renderPass;
		out.pipeline = pipeline;
		out.layout = layout;

		registerObjectForDeletion(layout);
		registerObjectForDeletion(_renderPass);
		registerObjectForDeletion(*pipelineInfoN.fragmentShader);
		registerObjectForDeletion(*pipelineInfoN.vertexShader);

		return out;
	}

	uint32_t Engine::registerPipeline(PipelineBundle pipeline)
	{
		uint32_t index = pipelineStorage.size();
		pipelineStorage.push_back(pipeline);
		if (pipeline != pipelineStorage[index])
		{
			throw std::exception("An unexpected error occured during registering the pipeline");
		}
		return index;
	}

	PipelineBundle* Engine::getPipeline(uint32_t index)
	{
		return &(pipelineStorage[index]);
	}

	//define in class
	vk::RenderPass Engine::CreateRenderPass(RenderPassInfo info, SwapChainBundle* swapchainBundle)
	{
		//!!! this is temporary
		swapchainBundle->depthFormat = vk::Format::eD32Sfloat;

		vk::RenderPass out;

		std::vector<vk::AttachmentDescription> attachments = {};

		//get the attachments

		uint64_t colorIndex = 0, depthIndex = 0, i = 0;

		for (Attachment& a : info.attachments)
		{
			if (a.type == AttachmentType::Color)
			{
				vk::AttachmentDescription colorAttachment = {};
				colorAttachment.flags = vk::AttachmentDescriptionFlags();
				colorAttachment.format = swapchainBundle->format;
				colorAttachment.samples = a.sampleCount;
				colorAttachment.loadOp = a.loadOp;
				colorAttachment.storeOp = a.storeOp;
				colorAttachment.stencilLoadOp = a.stencilLoadOp;
				colorAttachment.stencilStoreOp = a.stencilStoreOp;
				colorAttachment.initialLayout = a.initialLayout;
				colorAttachment.finalLayout = a.finalLayout;
				attachments.push_back(colorAttachment);
				colorIndex = i;
			}
			else if (a.type == AttachmentType::Depth)
			{
				vk::AttachmentDescription depthAttachment = {};
				depthAttachment.flags = vk::AttachmentDescriptionFlags();
				depthAttachment.format = swapchainBundle->depthFormat;
				depthAttachment.samples = a.sampleCount;
				depthAttachment.loadOp = a.loadOp;
				depthAttachment.storeOp = a.storeOp;
				depthAttachment.stencilLoadOp = a.stencilLoadOp;
				depthAttachment.stencilStoreOp = a.stencilStoreOp;
				depthAttachment.initialLayout = a.initialLayout;
				depthAttachment.finalLayout = a.finalLayout;
				attachments.push_back(depthAttachment);
				depthIndex = i;
			}
			i++;
		}

		vk::AttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = colorIndex;
		colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::AttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = depthIndex;
		depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::SubpassDescription subpass = {};
		subpass.flags = vk::SubpassDescriptionFlags();
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		vk::SubpassDependency dependency(
			VK_SUBPASS_EXTERNAL, 0,
			vk::PipelineStageFlagBits::eEarlyFragmentTests,
			vk::PipelineStageFlagBits::eLateFragmentTests,
			vk::AccessFlagBits::eDepthStencilAttachmentWrite,
			vk::AccessFlagBits::eDepthStencilAttachmentRead
		);

		vk::RenderPassCreateInfo renderpassInfo = {};
		renderpassInfo.flags = vk::RenderPassCreateFlags();
		renderpassInfo.attachmentCount = attachments.size();
		renderpassInfo.pAttachments = attachments.data();
		renderpassInfo.subpassCount = 1;
		renderpassInfo.pSubpasses = &subpass;
		renderpassInfo.dependencyCount = 1;
		renderpassInfo.pDependencies = &dependency;

		try {
			out = this->logicalDevice.createRenderPass(renderpassInfo);
		}
		catch (vk::SystemError err) {
			throw std::exception(err.what());
		}

		return out;
	}
}