#pragma once

#include "artcode.hpp"
#include <algorithm>
#include <cstdio>
#include <fcntl.h>
#include <glm/glm.hpp>
#include <sys/mman.h>
#include <unistd.h>
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

struct Vert {
    Vec2 element;
};

struct Indx {
    u32 element;
};

// TODO:create shared memory for api and app
namespace Shared {
    struct Instance {
        Vert vertex[999];
        Indx index[9999];
    };

    struct Region {
        size_t           count = 0;
        Shared::Instance instance[500];
    };

    struct Memory {
        static Shared::Region* region;

        static void init() {
            int fd = shm_open("/artcode_instances", O_CREAT | O_RDWR, 0666);
            ftruncate(fd, sizeof(Shared::Region));

            region = (Shared::Region*)mmap(nullptr, sizeof(Shared::Region),
                                           PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

            close(fd);

            // clear on first init
            if (region->count < 0 || region->count >= 500)
                region->count = 0;
        }

        static void cleanup() {
            munmap(region, sizeof(Shared::Region));
            shm_unlink("/artcode_instances");
        }

        static void register_instance(const size_t count, const ArrayVec2& vertex,
                                      const ArrayU32 index) {
            // max num of instances
            if (region->count >= 500)
                return;

            region->count = count;
            // insert vertex
            for (const auto& vert : vertex) {
                region->instance->vertex->element = vert;
            }
            // insert indices
            for (const auto& idx : index) {
                region->instance->index->element = idx;
            }
        }

        static Region get_instance() { return *region; }

        static Vert get_vertex() { return region->instance->vertex[region->count]; }

        static Indx get_index() { return region->instance->index[region->count]; }

        static void reset_instance() {
            region->count = 0;
            std::fill(std::begin(region->instance), std::end(region->instance),
                      Shared::Instance{});
        }
    };
} // namespace Shared

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
