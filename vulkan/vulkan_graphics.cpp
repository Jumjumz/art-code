#include "vulkan_graphics.hpp"
#include "vertex.hpp"
#include <fstream>

VulkanGraphics::VulkanGraphics(const vk::raii::PhysicalDevice &physical_device,
                               const vk::raii::Device &device,
                               const vk::Format &image_format,
                               const vk::Format &depth_format,
                               const int &graphics_family)
    : physical_device(physical_device), device(device),
      image_format(image_format), depth_format(depth_format),
      graphics_family(graphics_family) {
    create_descriptor_set_layout();
    create_graphics_pipeline();
    create_command_pool();
};

[[nodiscard]]
vk::raii::ShaderModule
VulkanGraphics::create_shader_module(const std::vector<char> &code) const {
    vk::ShaderModuleCreateInfo shader_info{};
    shader_info.codeSize = code.size();
    shader_info.pCode = reinterpret_cast<const uint32_t *>(code.data());

    vk::raii::ShaderModule vert_shader_module{this->device, shader_info, nullptr};

    return vert_shader_module;
};

std::vector<char> VulkanGraphics::read_file(const std::string &file_name) {
    std::ifstream file(file_name, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file!");

    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();

    return buffer;
};

void VulkanGraphics::create_descriptor_set_layout() {
    vk::DescriptorSetLayoutBinding ubo_layout_binding(
        0, vk::DescriptorType::eUniformBuffer, 1,
        vk::ShaderStageFlagBits::eVertex, nullptr);

    vk::DescriptorSetLayoutCreateInfo descriptor_info{};
    descriptor_info.bindingCount = 1;
    descriptor_info.pBindings = &ubo_layout_binding;

    this->descriptor_set_layout =
        vk::raii::DescriptorSetLayout{this->device, descriptor_info, nullptr};
};

void VulkanGraphics::create_graphics_pipeline() {
    this->vert_shader_module =
        create_shader_module(read_file("shaders/test.vert.spv"));
    this->frag_shader_module =
        create_shader_module(read_file("shaders/test.frag.spv"));

    vk::PipelineShaderStageCreateInfo vert_shader_stage_info{};
    vert_shader_stage_info.stage = vk::ShaderStageFlagBits::eVertex;
    vert_shader_stage_info.module = this->vert_shader_module;
    vert_shader_stage_info.pName = "main";

    vk::PipelineShaderStageCreateInfo frag_shader_stage_info{};
    frag_shader_stage_info.stage = vk::ShaderStageFlagBits::eFragment;
    frag_shader_stage_info.module = this->frag_shader_module;
    frag_shader_stage_info.pName = "main";

    vk::PipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info,
                                                         frag_shader_stage_info};

    vk::PipelineInputAssemblyStateCreateInfo assembly_info{};
    assembly_info.topology = vk::PrimitiveTopology::eLineList;

    std::vector<vk::DynamicState> dynamic_states = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor,
    };

    vk::PipelineDynamicStateCreateInfo dynamic_state_info{};
    dynamic_state_info.dynamicStateCount =
        static_cast<uint32_t>(dynamic_states.size());
    dynamic_state_info.pDynamicStates = dynamic_states.data();

    auto binding_description = Vertex::get_binding_description();
    auto attribute_description = Vertex::get_attribute_description();

    vk::PipelineVertexInputStateCreateInfo vertex_info{};
    vertex_info.vertexBindingDescriptionCount = 1;
    vertex_info.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attribute_description.size());
    vertex_info.pVertexBindingDescriptions = &binding_description;
    vertex_info.pVertexAttributeDescriptions = attribute_description.data();

    vk::PipelineViewportStateCreateInfo viewport_state_info{};
    viewport_state_info.pViewports = nullptr; // use dynamic viewport state
    viewport_state_info.pScissors = nullptr;  // use dunamic scissor state
    viewport_state_info.viewportCount = 1;
    viewport_state_info.scissorCount = 1;

    vk::PipelineRasterizationStateCreateInfo rasterization_state_info{};
    rasterization_state_info.depthClampEnable = vk::False;
    rasterization_state_info.rasterizerDiscardEnable = vk::False;
    rasterization_state_info.polygonMode = vk::PolygonMode::eFill;
    rasterization_state_info.cullMode = vk::CullModeFlagBits::eBack;
    rasterization_state_info.frontFace = vk::FrontFace::eCounterClockwise;
    rasterization_state_info.depthBiasEnable = vk::False;
    rasterization_state_info.depthBiasSlopeFactor = 1.0f;
    rasterization_state_info.lineWidth = 1.0f;

    vk::PipelineMultisampleStateCreateInfo multismapling_state_info{};
    multismapling_state_info.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multismapling_state_info.sampleShadingEnable = vk::False;

    vk::PipelineDepthStencilStateCreateInfo stencil_state_info{};
    stencil_state_info.depthTestEnable = vk::True;
    stencil_state_info.depthWriteEnable = vk::True;
    stencil_state_info.depthCompareOp = vk::CompareOp::eLess;
    stencil_state_info.depthBoundsTestEnable = vk::False;
    stencil_state_info.stencilTestEnable = vk::False;

    vk::PipelineColorBlendAttachmentState color_attachment{};
    color_attachment.blendEnable = vk::False;
    color_attachment.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    color_attachment.srcColorBlendFactor = vk::BlendFactor::eSrc1Alpha;
    color_attachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrc1Alpha;
    color_attachment.colorBlendOp = vk::BlendOp::eAdd;
    color_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    color_attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    color_attachment.alphaBlendOp = vk::BlendOp::eAdd;

    vk::PipelineColorBlendStateCreateInfo color_blend_info{};
    color_blend_info.logicOp = vk::LogicOp::eCopy;
    color_blend_info.attachmentCount = 1;
    color_blend_info.pAttachments = &color_attachment;

    vk::PipelineLayoutCreateInfo layout_info{};
    layout_info.setLayoutCount = 1;
    layout_info.pSetLayouts = &*this->descriptor_set_layout;
    layout_info.pushConstantRangeCount = 0;

    this->layout = vk::raii::PipelineLayout{this->device, layout_info, nullptr};

    vk::PipelineRenderingCreateInfo rendering_info{};
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachmentFormats = &this->image_format;
    rendering_info.depthAttachmentFormat = this->depth_format;

    vk::GraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pNext = &rendering_info;
    pipeline_info.pVertexInputState = &vertex_info;
    pipeline_info.pInputAssemblyState = &assembly_info;
    pipeline_info.pViewportState = &viewport_state_info;
    pipeline_info.pRasterizationState = &rasterization_state_info;
    pipeline_info.pMultisampleState = &multismapling_state_info;
    pipeline_info.pColorBlendState = &color_blend_info;
    pipeline_info.pDynamicState = &dynamic_state_info;
    pipeline_info.pDepthStencilState = &stencil_state_info;
    pipeline_info.layout = this->layout;
    pipeline_info.renderPass = nullptr;
    pipeline_info.basePipelineHandle = nullptr;
    pipeline_info.basePipelineIndex = -1;

    this->graphics_pipeline =
        vk::raii::Pipeline{this->device, nullptr, pipeline_info, nullptr};
};

void VulkanGraphics::create_command_pool() {
    vk::CommandPoolCreateInfo pool_info{};
    pool_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    pool_info.queueFamilyIndex = static_cast<uint32_t>(this->graphics_family);

    this->command_pool = vk::raii::CommandPool{this->device, pool_info, nullptr};
};
