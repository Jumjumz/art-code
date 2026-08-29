#include "artcode_graphics.hpp"
#include "artcode_instance.hpp"
#include <array>
#include <fstream>

ArtcodeGraphics::ArtcodeGraphics(const vk::raii::Device& device, vk::Format& image_format)
    : device(device),
      image_format(image_format) {
    create_descriptor_set_layout();
};

[[nodiscard]]
vk::raii::ShaderModule
ArtcodeGraphics::create_shader_module(const std::vector<char>& code) const {
    vk::ShaderModuleCreateInfo shader_info{};
    shader_info.codeSize = code.size();
    shader_info.pCode    = reinterpret_cast<const uint32_t*>(code.data());

    vk::raii::ShaderModule shader_module{this->device, shader_info, nullptr};

    return shader_module;
};

std::vector<char> ArtcodeGraphics::read_file(const std::string& file_name) const {
    std::ifstream file(file_name, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open the artcode shader files! " + file_name);

    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();

    return buffer;
};

void ArtcodeGraphics::create_descriptor_set_layout() {
    vk::DescriptorSetLayoutBinding ubo_layout_binding(
        0, vk::DescriptorType::eUniformBuffer, 1,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, nullptr);

    vk::DescriptorSetLayoutBinding ssbo_layout_binding(
        1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex,
        nullptr);

    std::array<vk::DescriptorSetLayoutBinding, 2> layout_bindings = {ubo_layout_binding,
                                                                     ssbo_layout_binding};

    vk::DescriptorSetLayoutCreateInfo descriptor_info{};
    descriptor_info.bindingCount = layout_bindings.size();
    descriptor_info.pBindings    = layout_bindings.data();

    this->artcode_set_layout =
        vk::raii::DescriptorSetLayout{this->device, descriptor_info, nullptr};
};

void ArtcodeGraphics::create_shaders() {
    // create shaders
    this->vert_shader_module =
        create_shader_module(read_file("shaders/artcode.vert.spv"));
    this->frag_shader_module =
        create_shader_module(read_file("shaders/artcode.frag.spv"));

    vk::PipelineShaderStageCreateInfo vert_shader_stage_info{};
    vert_shader_stage_info.stage  = vk::ShaderStageFlagBits::eVertex;
    vert_shader_stage_info.module = this->vert_shader_module;
    vert_shader_stage_info.pName  = "main";

    vk::PipelineShaderStageCreateInfo frag_shader_stage_info{};
    frag_shader_stage_info.stage  = vk::ShaderStageFlagBits::eFragment;
    frag_shader_stage_info.module = this->frag_shader_module;
    frag_shader_stage_info.pName  = "main";

    this->shader_stages = {vert_shader_stage_info, frag_shader_stage_info};
};

void ArtcodeGraphics::create_pipeline() {
    vk::PipelineInputAssemblyStateCreateInfo assembly_info{};
    assembly_info.topology = vk::PrimitiveTopology::eTriangleList;

    std::array<vk::DynamicState, 2> dynamic_states = {vk::DynamicState::eViewport,
                                                      vk::DynamicState::eScissor};

    vk::PipelineDynamicStateCreateInfo dynamic_state_info{};
    dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state_info.pDynamicStates    = dynamic_states.data();

    // vert and index bindings
    const auto& binding_desc   = Vertex::get_binding_description();
    const auto& attribute_desc = Vertex::get_attribute_description();

    vk::PipelineVertexInputStateCreateInfo vertex_info{};
    vertex_info.vertexBindingDescriptionCount = 1;
    vertex_info.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attribute_desc.size());
    vertex_info.pVertexBindingDescriptions   = &binding_desc;
    vertex_info.pVertexAttributeDescriptions = attribute_desc.data();

    vk::PipelineViewportStateCreateInfo viewport_state_info{};
    viewport_state_info.pViewports    = nullptr; // use dynamic viewport state
    viewport_state_info.pScissors     = nullptr; // use dunamic scissor state
    viewport_state_info.viewportCount = 1;
    viewport_state_info.scissorCount  = 1;

    vk::PipelineRasterizationStateCreateInfo rasterization_state_info{};
    rasterization_state_info.depthClampEnable = vk::False;
    rasterization_state_info.polygonMode      = vk::PolygonMode::eFill;
    rasterization_state_info.cullMode         = vk::CullModeFlagBits::eNone;
    rasterization_state_info.lineWidth        = 1.0f;

    vk::PipelineMultisampleStateCreateInfo multismapling_state_info{};
    multismapling_state_info.rasterizationSamples = vk::SampleCountFlagBits::e4;
    multismapling_state_info.sampleShadingEnable  = vk::False;
    // multismapling_state_info.minSampleShading     = 0.2f;

    vk::PipelineDepthStencilStateCreateInfo stencil_state_info{};
    stencil_state_info.depthTestEnable       = vk::False;
    stencil_state_info.depthWriteEnable      = vk::False;
    stencil_state_info.depthBoundsTestEnable = vk::False;
    stencil_state_info.stencilTestEnable     = vk::False;

    vk::PipelineColorBlendAttachmentState color_attachment{};
    color_attachment.blendEnable         = vk::True;
    color_attachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    color_attachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    color_attachment.colorBlendOp        = vk::BlendOp::eAdd;
    color_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    color_attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    color_attachment.alphaBlendOp        = vk::BlendOp::eAdd;
    color_attachment.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    vk::PipelineColorBlendStateCreateInfo blend_info{};
    blend_info.logicOp         = vk::LogicOp::eCopy;
    blend_info.attachmentCount = 1;
    blend_info.pAttachments    = &color_attachment;

    vk::PipelineRenderingCreateInfo rendering_info{};
    rendering_info.colorAttachmentCount    = 1;
    rendering_info.pColorAttachmentFormats = &this->image_format;

    // TODO:instead of passing the entire push constants to both vert and frag stages,
    //  use push constant stages to only use what each stages needs
    //  push constants
    vk::PushConstantRange constant_range{};
    constant_range.stageFlags =
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
    constant_range.offset = 0;
    constant_range.size   = sizeof(PushConstants);

    vk::PipelineLayoutCreateInfo layout_info{};
    layout_info.setLayoutCount         = 1;
    layout_info.pSetLayouts            = &*this->artcode_set_layout;
    layout_info.pushConstantRangeCount = 1;
    layout_info.pPushConstantRanges    = &constant_range;

    this->layout = vk::raii::PipelineLayout{this->device, layout_info, nullptr};

    vk::GraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.stageCount          = this->shader_stages.size();
    pipeline_info.pStages             = this->shader_stages.data();
    pipeline_info.pNext               = &rendering_info;
    pipeline_info.pVertexInputState   = &vertex_info;
    pipeline_info.pInputAssemblyState = &assembly_info;
    pipeline_info.pViewportState      = &viewport_state_info;
    pipeline_info.pRasterizationState = &rasterization_state_info;
    pipeline_info.pMultisampleState   = &multismapling_state_info;
    pipeline_info.pColorBlendState    = &blend_info;
    pipeline_info.pDynamicState       = &dynamic_state_info;
    pipeline_info.pDepthStencilState  = &stencil_state_info;
    pipeline_info.layout              = this->layout;
    pipeline_info.renderPass          = nullptr;
    pipeline_info.basePipelineHandle  = nullptr;
    pipeline_info.basePipelineIndex   = -1;

    // create pipeline
    this->pipeline_trianglelist =
        vk::raii::Pipeline{this->device, nullptr, pipeline_info, nullptr};
};
