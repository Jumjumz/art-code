#pragma once

#include "artcode.hpp"
#include <algorithm>
#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;

    static vk::VertexInputBindingDescription getBindingDescription() {
        return {0, sizeof(Vertex), vk::VertexInputRate::eVertex};
    };

    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescription() {
        return {
            vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat,
                                                offsetof(Vertex, pos)},
            vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32Sfloat,
                                                offsetof(Vertex, color)},
        };
    };
};

typedef detail::IPen ArtInstance;

struct ArtcodeInstance {
  public:
    static inline void register_instance(ArtInstance* instance) {
        if (ArtcodeInstance::instance.size() == 0)
            ArtcodeInstance::instance.reserve(20);

        ArtcodeInstance::instance.push_back(instance);
    }

    static inline std::vector<ArtInstance*> get_instance() {
        return ArtcodeInstance::instance;
    }

    static inline void delete_instance(ArtInstance* instance) {
        // iterator
        const auto it = std::remove(ArtcodeInstance::instance.begin(),
                                    ArtcodeInstance::instance.end(), instance);
        ArtcodeInstance::instance.erase(it, ArtcodeInstance::instance.end());
    }

  private:
    static inline std::vector<ArtInstance*> instance;
};
