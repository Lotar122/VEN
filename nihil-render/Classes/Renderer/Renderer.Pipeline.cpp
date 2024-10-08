#include "Renderer.hpp"

#include "Classes/Engine/Engine.hpp"

namespace nihil::graphics {
	void deleteShaderModule(void* x)
	{
		delete (vk::ShaderModule*)x;
	}

	void Renderer::CreateShaderModule(std::string filepath, vk::Device device, vk::ShaderModule** ppShaderModule)
	{
		std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << MAGENTA << "->" << YELLOW << "[Create-Shader]" << RESET << "Creating shader module ";
		std::vector<char> sourceCode = SPIRV::LoadSPVFile(filepath);
		vk::ShaderModuleCreateInfo createInfo = {};
		createInfo.flags = vk::ShaderModuleCreateFlags();
		createInfo.codeSize = sourceCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(sourceCode.data());

		vk::ShaderModule* module = (vk::ShaderModule*)engine->shaderManager.allocate();
		*module = vk::ShaderModule();

		try {
			*module = device.createShaderModule(createInfo);
		}
		catch (vk::SystemError err) {
			std::cerr << RED << "[###]" << RESET << std::endl;
			engine->error = true;
			engine->finishPipelineSetup();
			engine->finishSetup();
			throw std::exception(err.what());
		}
		std::cout << GREEN << "[###]" << RESET << std::endl;

		*ppShaderModule = module;
	}

	//void Renderer::PipelineSetup()
	//{
	//	vk::GraphicsPipelineCreateInfo pipelineInfo = {};
	//	pipelineInfo.flags = vk::PipelineCreateFlags();
	//	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

	//	/*
	//	* create bindings for a mat4 by making 4 vec4, make sure to add offsets
	//	*/

	//	if (engine->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Creating a vertex input " << GREEN << "[###]" << RESET << std::endl;
	//	//Vertex Input
	//	std::array<vk::VertexInputBindingDescription, 2> bindingDesc;

	//	bindingDesc[0].binding = 0;
	//	bindingDesc[0].stride = 11 * sizeof(float);
	//	bindingDesc[0].inputRate = vk::VertexInputRate::eVertex;

	//	bindingDesc[1].binding = 1;
	//	bindingDesc[1].stride = 16 * sizeof(float);
	//	bindingDesc[1].inputRate = vk::VertexInputRate::eInstance;

	//	std::array<vk::VertexInputAttributeDescription, 8> attributes;

	//	attributes[0].binding = 0;
	//	attributes[0].location = 0;
	//	attributes[0].format = vk::Format::eR32G32B32Sfloat;
	//	attributes[0].offset = 0;

	//	attributes[1].binding = 0;
	//	attributes[1].location = 1;
	//	attributes[1].format = vk::Format::eR32G32B32Sfloat;
	//	attributes[1].offset = 3 * sizeof(float);

	//	attributes[2].binding = 0;
	//	attributes[2].location = 2;
	//	attributes[2].format = vk::Format::eR32G32Sfloat;
	//	attributes[2].offset = 6 * sizeof(float);

	//	attributes[3].binding = 0;
	//	attributes[3].location = 3;
	//	attributes[3].format = vk::Format::eR32G32B32Sfloat;
	//	attributes[3].offset = 8 * sizeof(float);


	//	attributes[4].binding = 1;
	//	attributes[4].location = 5;
	//	attributes[4].format = vk::Format::eR32G32B32A32Sfloat;
	//	attributes[4].offset = 0;

	//	attributes[5].binding = 1;
	//	attributes[5].location = 6;
	//	attributes[5].format = vk::Format::eR32G32B32A32Sfloat;
	//	attributes[5].offset = 4 * sizeof(float);

	//	attributes[6].binding = 1;
	//	attributes[6].location = 7;
	//	attributes[6].format = vk::Format::eR32G32B32A32Sfloat;
	//	attributes[6].offset = 8 * sizeof(float);

	//	attributes[7].binding = 1;
	//	attributes[7].location = 8;
	//	attributes[7].format = vk::Format::eR32G32B32A32Sfloat;
	//	attributes[7].offset = 12 * sizeof(float);

	//	vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
	//	vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
	//	vertexInputInfo.vertexBindingDescriptionCount = bindingDesc.size();
	//	vertexInputInfo.pVertexBindingDescriptions = bindingDesc.data();
	//	vertexInputInfo.vertexAttributeDescriptionCount = attributes.size();
	//	vertexInputInfo.pVertexAttributeDescriptions = attributes.data();
	//	pipelineInfo.pVertexInputState = &vertexInputInfo;

	//	if (engine->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Creating a input assembly " << GREEN << "[###]" << RESET << std::endl;
	//	//Input Assembly
	//	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
	//	inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
	//	inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
	//	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;

	//	//Vertex shader
	//	// in class declaration: vk::ShaderModule* vertexShader = NULL;
	//	CreateShaderModule("./resources/Shaders/shaderV.spv", engine->logicalDevice, &vertexShader);
	//	vk::PipelineShaderStageCreateInfo vertexShaderInfo = {};
	//	vertexShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
	//	vertexShaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
	//	vertexShaderInfo.module = *vertexShader;
	//	vertexShaderInfo.pName = "main";
	//	shaderStages.push_back(vertexShaderInfo);

	//	if (engine->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Creating the viewport " << GREEN << "[###]" << RESET << std::endl;
	//	//Viewport
	//	viewport.x = 0.0f;
	//	viewport.y = 0.0f;
	//	viewport.width = swapchainBundle.extent.width;
	//	viewport.height = swapchainBundle.extent.height;
	//	viewport.minDepth = 0.0f;
	//	viewport.maxDepth = 1.0f;
	//	
	//	scissor.offset.x = 0.0f;
	//	scissor.offset.y = 0.0f;
	//	scissor.extent = swapchainBundle.extent;
	//	vk::PipelineViewportStateCreateInfo viewportInfo = {};
	//	viewportInfo.flags = vk::PipelineViewportStateCreateFlags();
	//	viewportInfo.viewportCount = 1;
	//	viewportInfo.pViewports = &viewport;
	//	viewportInfo.scissorCount = 1;
	//	viewportInfo.pScissors = &scissor;

	//	// Define dynamic states
	//	std::vector<vk::DynamicState> dynamicStates = {
	//		vk::DynamicState::eViewport,
	//		vk::DynamicState::eScissor
	//	};

	//	// Create pipeline dynamic state info
	//	vk::PipelineDynamicStateCreateInfo dynamicStateInfo(
	//		vk::PipelineDynamicStateCreateFlags(),
	//		dynamicStates.size(),
	//		dynamicStates.data()
	//	);

	//	pipelineInfo.pViewportState = &viewportInfo;
	//	pipelineInfo.pDynamicState = &dynamicStateInfo;

	//	if (engine->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Creating a rasterizer " << GREEN << "[###]" << RESET << std::endl;
	//	//Rasterization
	//	vk::PipelineRasterizationStateCreateInfo rasterizationInfo = {};
	//	rasterizationInfo.flags = vk::PipelineRasterizationStateCreateFlags();
	//	rasterizationInfo.depthClampEnable = VK_FALSE;
	//	rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	//	rasterizationInfo.polygonMode = vk::PolygonMode::eFill;
	//	rasterizationInfo.lineWidth = 1.0f;
	//	rasterizationInfo.cullMode = vk::CullModeFlagBits::eNone;
	//	//make editable
	//	rasterizationInfo.frontFace = vk::FrontFace::eCounterClockwise;
	//	rasterizationInfo.depthBiasEnable = VK_FALSE;
	//	pipelineInfo.pRasterizationState = &rasterizationInfo;

	//	//Fragment shader
	//	//in class declaration: vk::ShaderModule* fragmentShader = NULL;
	//	CreateShaderModule("./resources/Shaders/shaderF.spv", engine->logicalDevice, &fragmentShader);
	//	vk::PipelineShaderStageCreateInfo fragmentShaderInfo = {};
	//	fragmentShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
	//	fragmentShaderInfo.stage = vk::ShaderStageFlagBits::eFragment;
	//	fragmentShaderInfo.module = *fragmentShader;
	//	fragmentShaderInfo.pName = "main";
	//	shaderStages.push_back(fragmentShaderInfo);

	//	if (engine->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Setting shader stages " << GREEN << "[###]" << RESET << std::endl;
	//	//Set shaders
	//	pipelineInfo.stageCount = shaderStages.size();
	//	pipelineInfo.pStages = shaderStages.data();

	//	if (engine->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Creating a multisampler " << GREEN << "[###]" << RESET << std::endl;
	//	//Multisampling
	//	vk::PipelineMultisampleStateCreateInfo multisampleInfo = {};
	//	multisampleInfo.flags = vk::PipelineMultisampleStateCreateFlags();
	//	multisampleInfo.sampleShadingEnable = VK_FALSE;
	//	multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
	//	pipelineInfo.pMultisampleState = &multisampleInfo;

	//	if (engine->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Specifying a color blender " << GREEN << "[###]" << RESET << std::endl;
	//	//Color blend
	//	vk::PipelineColorBlendAttachmentState attachment = {};
	//	attachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	//	attachment.blendEnable = VK_FALSE;
	//	vk::PipelineColorBlendStateCreateInfo colorBlendingInfo = {};
	//	colorBlendingInfo.flags = vk::PipelineColorBlendStateCreateFlags();
	//	colorBlendingInfo.logicOpEnable = VK_FALSE;
	//	colorBlendingInfo.logicOp = vk::LogicOp::eCopy;
	//	colorBlendingInfo.attachmentCount = 1;
	//	colorBlendingInfo.pAttachments = &attachment;
	//	colorBlendingInfo.blendConstants[0] = 0.0f;
	//	colorBlendingInfo.blendConstants[1] = 0.0f;
	//	colorBlendingInfo.blendConstants[2] = 0.0f;
	//	colorBlendingInfo.blendConstants[3] = 0.0f;
	//	pipelineInfo.pColorBlendState = &colorBlendingInfo;

	//	vk::PipelineDepthStencilStateCreateInfo depthInfo = {};
	//	depthInfo.depthTestEnable = VK_TRUE;
	//	depthInfo.depthWriteEnable = VK_TRUE;
	//	depthInfo.depthCompareOp = vk::CompareOp::eLess;

	//	pipelineInfo.pDepthStencilState = &depthInfo;

	//	if (engine->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Creating a pipeline layout ";
	//	//Create pipeline layout (move to a seperate function in future)
	//	//in class declaration: vk::PipelineLayout layout;
	//	vk::PipelineLayoutCreateInfo layoutInfo = {};
	//	layoutInfo.flags = vk::PipelineLayoutCreateFlags();
	//	layoutInfo.setLayoutCount = 0;
	//	layoutInfo.pushConstantRangeCount = 1;

	//	vk::PushConstantRange pushConstantInfo = {};
	//	pushConstantInfo.offset = 0;
	//	pushConstantInfo.size = sizeof(ObjectData);
	//	pushConstantInfo.stageFlags = vk::ShaderStageFlagBits::eVertex;

	//	layoutInfo.pPushConstantRanges = &pushConstantInfo;

	//	try {
	//		layout = engine->logicalDevice.createPipelineLayout(layoutInfo);
	//	}
	//	catch (vk::SystemError) {
	//		std::cout << RED << "[###]" << RESET << std::endl;
	//		engine->error = true;
	//		engine->finishPipelineSetup();
	//		std::abort();
	//	}
	//	std::cout << GREEN << "[###]" << RESET << std::endl;

	//	if (engine->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Creating a renderpass ";
	//	//Create render pass (move to a seperate function in future)
	//	//in class declaration: vk::RenderPass renderPass;
	//	vk::AttachmentDescription colorAttachment = {};
	//	colorAttachment.flags = vk::AttachmentDescriptionFlags();
	//	colorAttachment.format = swapchainBundle.format;
	//	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	//	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	//	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	//	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	//	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	//	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	//	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	//	renderPassAttachments.push_back(colorAttachment);

	//	swapchainBundle.depthFormat = vk::Format::eD32Sfloat;

	//	vk::AttachmentDescription depthAttachment = {};
	//	depthAttachment.format = swapchainBundle.depthFormat;
	//	depthAttachment.samples = vk::SampleCountFlagBits::e1;
	//	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	//	depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	//	depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	//	depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	//	depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	//	depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	//	renderPassAttachments.push_back(depthAttachment);

	//	vk::AttachmentReference colorAttachmentRef = {};
	//	colorAttachmentRef.attachment = 0;
	//	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	//	vk::AttachmentReference depthAttachmentRef = {};
	//	depthAttachmentRef.attachment = 1;
	//	depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	//	vk::SubpassDescription subpass = {};
	//	subpass.flags = vk::SubpassDescriptionFlags();
	//	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	//	subpass.colorAttachmentCount = 1;
	//	subpass.pColorAttachments = &colorAttachmentRef;
	//	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	//	vk::SubpassDependency dependency(
	//		VK_SUBPASS_EXTERNAL, 0,
	//		vk::PipelineStageFlagBits::eEarlyFragmentTests,
	//		vk::PipelineStageFlagBits::eLateFragmentTests,
	//		vk::AccessFlagBits::eDepthStencilAttachmentWrite,
	//		vk::AccessFlagBits::eDepthStencilAttachmentRead
	//	);

	//	vk::RenderPassCreateInfo renderpassInfo = {};
	//	renderpassInfo.flags = vk::RenderPassCreateFlags();
	//	renderpassInfo.attachmentCount = renderPassAttachments.size();
	//	renderpassInfo.pAttachments = renderPassAttachments.data();
	//	renderpassInfo.subpassCount = 1;
	//	renderpassInfo.pSubpasses = &subpass;
	//	renderpassInfo.dependencyCount = 1;
	//	renderpassInfo.pDependencies = &dependency;

	//	try {
	//		renderPass = engine->logicalDevice.createRenderPass(renderpassInfo);
	//	}
	//	catch (vk::SystemError) {
	//		std::cerr << RED << "[###]" << RESET << std::endl;
	//		engine->error = true;
	//		engine->finishPipelineSetup();
	//		std::abort();
	//	}
	//	std::cout << GREEN << "[###]" << RESET << std::endl;

	//	//finish pipeline creation
	//	pipelineInfo.layout = layout;
	//	pipelineInfo.renderPass = renderPass;
	//	pipelineInfo.basePipelineHandle = nullptr;

	//	try {
	//		pipeline = engine->logicalDevice.createGraphicsPipeline(nullptr, pipelineInfo).value;
	//	}
	//	catch (vk::SystemError err) {
	//		engine->error = true;
	//		engine->finishPipelineSetup();
	//		std::abort();
	//	}
	//	engine->finishPipelineSetup();
	//}
}