#include "Pipeline.hpp"
#include "Classes/DescriptorSet/DescriptorSet.hpp"

using namespace nihil::graphics;

Pipeline::Pipeline(Engine* _engine)
{
	assert(_engine != nullptr);

	engine = _engine;
}

void Pipeline::destroy()
{
	assert(!destroyed);

	destroyed = true;
	pipeline.destroy();
	layout.destroy();

	Carbo::Logger::Log("Destroyed Pipeline.");
}

Pipeline::~Pipeline()
{
	if(!destroyed) destroy();
}

void Pipeline::create(PipelineCreateInfo& info, RenderPass* _renderPass, DescriptorAllocator* descriptorAllocator)
{
	assert(_renderPass != nullptr);
	assert(info.vertexShader != nullptr);
	assert(info.fragmentShader != nullptr);

	baseRenderPass = _renderPass;

	assert(info.descriptorSetLayoutBindings.size() == 0 || descriptorAllocator != nullptr);

	std::vector<const DescriptorSetLayoutBinding*> dynamicDescriptors;
	dynamicDescriptors.reserve(info.descriptorSetLayoutBindings.size());

	std::vector<const DescriptorSetLayoutBinding*> staticDescriptors;
	staticDescriptors.reserve(info.descriptorSetLayoutBindings.size());

	for(const DescriptorSetLayoutBinding& dsb : info.descriptorSetLayoutBindings)
	{
		if (dsb.usage == AssetUsage::Static)
		{
			auto it = descriptorAllocator->globalDescriptorSet->descriptors.find(dsb.layoutBinding.binding);
			if (it == descriptorAllocator->globalDescriptorSet->descriptors.end())
			{
				Carbo::Logger::Exception("The static descriptor (binding = {}) passed to the pipeline is not present in the descriptor set", dsb.layoutBinding.binding);
			}
			if (it->second.descriptorInfo.type != dsb.descriptorInfo.type)
			{
				Carbo::Logger::Exception("The static descriptor (binding = {}) passed to the pipeline is of the wrong type", dsb.layoutBinding.binding);
			}

			staticDescriptors.push_back(&dsb);
		}
		else dynamicDescriptors.push_back(&dsb);
	}

	//Dynamic Descriptor set


	//?Pipeline creation

	vk::GraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.flags = vk::PipelineCreateFlags();
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

	//Vertex Input
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(info.bindingDesc.size());
	vertexInputInfo.pVertexBindingDescriptions = info.bindingDesc.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(info.attributeDesc.size());
	vertexInputInfo.pVertexAttributeDescriptions = info.attributeDesc.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;

	//Input Assembly
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
	inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
	inputAssemblyInfo.topology = info.topology;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;

	//Vertex shader
	vk::PipelineShaderStageCreateInfo vertexShaderInfo = {};
	vertexShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
	vertexShaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
	vertexShaderInfo.module = *info.vertexShader;
	vertexShaderInfo.pName = "main";
	shaderStages.push_back(vertexShaderInfo);

	//*Viewport stuff
	vk::PipelineViewportStateCreateInfo viewportInfo = {};
	viewportInfo.flags = vk::PipelineViewportStateCreateFlags();
	viewportInfo.viewportCount = 1;
	viewportInfo.pViewports = &engine->_renderer()->_viewport();
	viewportInfo.scissorCount = 1;
	viewportInfo.pScissors = &engine->_renderer()->_scissor();

	// Define dynamic states
	std::vector<vk::DynamicState> dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};

	// Create pipeline dynamic state info
	vk::PipelineDynamicStateCreateInfo dynamicStateInfo(
		vk::PipelineDynamicStateCreateFlags(),
		static_cast<uint32_t>(dynamicStates.size()),
		dynamicStates.data()
	);

	pipelineInfo.pViewportState = &viewportInfo;
	pipelineInfo.pDynamicState = &dynamicStateInfo;

	//Rasterization
	vk::PipelineRasterizationStateCreateInfo rasterizationInfo = {};
	rasterizationInfo.flags = vk::PipelineRasterizationStateCreateFlags();
	rasterizationInfo.depthClampEnable = info.depthClampEnable;
	rasterizationInfo.rasterizerDiscardEnable = info.rasterizerDiscardEnable;
	rasterizationInfo.polygonMode = info.polygonMode;
	rasterizationInfo.lineWidth = 1.0f;
	rasterizationInfo.cullMode = info.cullingMode;
	rasterizationInfo.frontFace = info.frontFace;
	rasterizationInfo.depthBiasEnable = info.depthBiasEnable;
	pipelineInfo.pRasterizationState = &rasterizationInfo;

	//Fragment shader
	vk::PipelineShaderStageCreateInfo fragmentShaderInfo = {};
	fragmentShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
	fragmentShaderInfo.stage = vk::ShaderStageFlagBits::eFragment;
	fragmentShaderInfo.module = *info.fragmentShader;
	fragmentShaderInfo.pName = "main";
	shaderStages.push_back(fragmentShaderInfo);

	//Set shaders
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();

	//Multisampling
	vk::PipelineMultisampleStateCreateInfo multisampleInfo = {};
	multisampleInfo.flags = vk::PipelineMultisampleStateCreateFlags();
	multisampleInfo.sampleShadingEnable = info.sampleShadingEnable;
	multisampleInfo.rasterizationSamples = info.rasterizationSampleCount;
	pipelineInfo.pMultisampleState = &multisampleInfo;

	//Color blending
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

	//Depth
	vk::PipelineDepthStencilStateCreateInfo depthInfo = {};
	depthInfo.depthTestEnable = VK_TRUE;
	depthInfo.depthWriteEnable = VK_TRUE;
	depthInfo.depthCompareOp = vk::CompareOp::eLess;
	
	pipelineInfo.pDepthStencilState = &depthInfo;

	//Push constants and pipeline layout creation
	vk::PipelineLayoutCreateInfo layoutInfo = {};
	layoutInfo.flags = vk::PipelineLayoutCreateFlags();
	layoutInfo.setLayoutCount = 0;
	layoutInfo.pushConstantRangeCount = 1;
	std::array<vk::DescriptorSetLayout, 1> descriptorSetLayouts;
	if (!descriptorAllocator)
	{
		layoutInfo.setLayoutCount = 0;
		layoutInfo.pSetLayouts = nullptr;
	}
	else
	{
		//the last element is ignored since the size is set to 1
		descriptorSetLayouts = { descriptorAllocator->globalDescriptorSet->layout };
		//for now 1 since there are no dynamic descriptors
		layoutInfo.setLayoutCount = 1;
		layoutInfo.pSetLayouts = descriptorSetLayouts.data();
	}

	vk::PushConstantRange pushConstantInfo = {};
	pushConstantInfo.offset = 0;

	pushConstantInfo.size = sizeof(PushConstants);
		
	pushConstantInfo.stageFlags = vk::ShaderStageFlagBits::eVertex;

	layoutInfo.pPushConstantRanges = &pushConstantInfo;

	try {
		layout.assignRes(engine->_device().createPipelineLayout(layoutInfo), engine->_device());
	}
	catch (vk::SystemError err) {
		Carbo::Logger::Exception(err.what());
	}

	pipelineInfo.layout = layout.getRes();
	pipelineInfo.renderPass = _renderPass->_renderPass();
	pipelineInfo.basePipelineHandle = nullptr;

	try {
		pipeline.assignRes(engine->_device().createGraphicsPipeline(engine->_pipelineCache(), pipelineInfo).value, engine->_device());
	}
	catch (vk::SystemError err) {
		Carbo::Logger::Exception(err.what());
	}

	Carbo::Logger::Log("Created the pipeline");
}