#include "Pipeline.h"

#include <fmt/format.h>

#include "gfx/vk/gpu_resources/Shader.h"
#include "gfx/vk/types/ModelConstants.h"
#include "gfx/vk/types/Vertex.h"
#include "gfx/vk/types/VulkanContext.h"
#include "gfx/vk/vkutil.h"

// Pipeline::Pipeline(const char* vertexShaderPath, const char* fragmentShaderPath,
//                    const VkPipelineVertexInputStateCreateInfo& vertexInputState,
//                    const VkPipelineInputAssemblyStateCreateInfo& inputAssembly,
//                    const VkPipelineViewportStateCreateInfo& viewportState,
//                    const VkPipelineRasterizationStateCreateInfo& rasterizer,
//                    const VkPipelineMultisampleStateCreateInfo& multisample,
//                    const VkPipelineColorBlendStateCreateInfo& colorBlendState,
//                    const VkPipelineDepthStencilStateCreateInfo& depthStencilState, const VkPipelineLayout& layout,
//                    const VkRenderPass& renderPass)
//     : m_vertexShader(vertexShaderPath, Shader::Type::Vertex),
//       m_fragmentShader(fragmentShaderPath, Shader::Type::Fragment) {
//     VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
//     vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//     vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//     vertShaderStageInfo.module = m_vertexShader.getModule();
//     vertShaderStageInfo.pName = m_vertexShader.getEntryPoint();
//
//     VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
//     fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//     fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//     fragShaderStageInfo.module = m_fragmentShader.getModule();
//     fragShaderStageInfo.pName = m_fragmentShader.getEntryPoint();
//
//     VkPipelineShaderStageCreateInfo shaderStages[] = {
//         vertShaderStageInfo,
//         fragShaderStageInfo,
//     };
//
//     VkGraphicsPipelineCreateInfo pipelineInfo{};
//     pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//     pipelineInfo.stageCount = 2;
//     pipelineInfo.pStages = shaderStages;
//     pipelineInfo.pVertexInputState = &vertexInputState;
//     pipelineInfo.pInputAssemblyState = &inputAssembly;
//     pipelineInfo.pViewportState = &viewportState;
//     pipelineInfo.pRasterizationState = &rasterizer;
//     pipelineInfo.pMultisampleState = &multisample;
//     pipelineInfo.pDepthStencilState = nullptr;
//     pipelineInfo.pColorBlendState = &colorBlendState;
//     pipelineInfo.pDepthStencilState = &depthStencilState;
//     // TODO: Later maybe
//     // pipelineInfo.pDynamicState = &dynamicStateInfo;
//     pipelineInfo.layout = layout;
//     pipelineInfo.renderPass = renderPass;
//     pipelineInfo.subpass = 0;
//
//     // TODO: Use pipeline cache?
//     VK_CHECK("failed to create graphics pipeline!",
//              vkCreateGraphicsPipelines(VulkanContext::get().getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
//                                        &m_underlying));
// }

const VkPipeline& Pipeline::getUnderlying() const {
    return m_underlying;
}

const VkPipelineLayout& Pipeline::getLayout() const {
    return m_layout;
}

//
// void Pipeline::bind(const VkCommandBuffer& commandBuffer) const {
//     vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_underlying);
// }

Pipeline::Pipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
    : m_vertexShader(std::make_unique<Shader>(Shader::Type::Vertex, vertexShaderPath)),
      m_fragmentShader(std::make_unique<Shader>(Shader::Type::Fragment, fragmentShaderPath)) {
    // VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
    // std::array attributeDescriptions = Vertex::getAttributeDescriptions();

    // VkPipelineVertexInputStateCreateInfo vtxInputInfo{};
    // vtxInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    // vtxInputInfo.vertexBindingDescriptionCount = 1;
    // vtxInputInfo.pVertexBindingDescriptions = &bindingDescription;
    // vtxInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    // vtxInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    // inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    // inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // VkPipelineRasterizationStateCreateInfo rasterizer{};
    // rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // rasterizer.depthClampEnable = VK_FALSE;
    // rasterizer.rasterizerDiscardEnable = VK_FALSE;
    // rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    // rasterizer.lineWidth = 1.0f;
    // rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    // rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    // rasterizer.depthBiasEnable = VK_FALSE;

    // VkPipelineMultisampleStateCreateInfo multisampling{};
    // multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    // multisampling.sampleShadingEnable = VK_FALSE;
    // multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    // colorBlendAttachment.colorWriteMask =
    //     VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    // colorBlendAttachment.blendEnable = VK_TRUE;
    // colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    // colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    // colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    // colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    // colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    // colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_SUBTRACT;
    //
    // VkPipelineColorBlendStateCreateInfo colorBlending{};
    // colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    // colorBlending.logicOpEnable = VK_FALSE;
    // colorBlending.attachmentCount = 1;
    // colorBlending.pAttachments = &colorBlendAttachment;

    // VkPipelineDepthStencilStateCreateInfo depthStencil{};
    // depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    // depthStencil.depthTestEnable = VK_TRUE;
    // depthStencil.depthWriteEnable = VK_TRUE;
    // depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

    // VkPushConstantRange pushConstant{};
    // pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    // pushConstant.offset = 0;
    // pushConstant.size = sizeof(ModelConstants);
    //
    // VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    // pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // pipelineLayoutInfo.setLayoutCount = setLayouts.size();
    // pipelineLayoutInfo.pSetLayouts = setLayouts.data();
    // pipelineLayoutInfo.pushConstantRangeCount = 1;
    // pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
}

VkPipelineVertexInputStateCreateInfo Pipeline::getVertexInputState() const {
    const auto bindingDescription = Vertex::getBindingDescription();
    const auto attributeDescriptions = Vertex::getAttributeDescriptions();

    const VkPipelineVertexInputStateCreateInfo vtxInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions->size()),
        .pVertexAttributeDescriptions = attributeDescriptions->data(),
    };

    return vtxInputInfo;
}

VkPipelineViewportStateCreateInfo Pipeline::getViewportState(const VkExtent2D& extent) const {
    static const VkViewport viewport{
        .width = static_cast<float>(extent.width),
        .height = static_cast<float>(extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    static const VkRect2D scissor{ { 0, 0 }, extent};
    const VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    return viewportState;
}

VkPipelineInputAssemblyStateCreateInfo Pipeline::getInputAssembly() const {
    constexpr VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    };

    return inputAssembly;
}

VkPipelineRasterizationStateCreateInfo Pipeline::getRasterizer() const {
    constexpr VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
    };

    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo Pipeline::getMultisampler() const {
    constexpr VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
    };

    return multisampling;
}

VkPipelineColorBlendStateCreateInfo Pipeline::getColorBlending() const {
    static constexpr VkPipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alphaBlendOp = VK_BLEND_OP_SUBTRACT,
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    static const VkPipelineColorBlendStateCreateInfo colorBlending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };

    return colorBlending;
}

VkPipelineDepthStencilStateCreateInfo Pipeline::getDepthStencil() const {
    constexpr VkPipelineDepthStencilStateCreateInfo depthStencil{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
    };

    return depthStencil;
}

void Pipeline::createLayout(const std::span<VkDescriptorSetLayout>& setLayouts) {
    static constexpr VkPushConstantRange pushConstant{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(ModelConstants),
    };

    const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
        .pSetLayouts = setLayouts.data(),
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstant,
    };

    VK_CHECK("Failed to create pipeline layout!",
             vkCreatePipelineLayout(VulkanContext::get().getDevice(), &pipelineLayoutInfo, nullptr, &m_layout));
}

void Pipeline::destroy() const {
    m_vertexShader->destroy();
    m_fragmentShader->destroy();

    vkDestroyPipeline(VulkanContext::get().getDevice(), m_underlying, nullptr);
}