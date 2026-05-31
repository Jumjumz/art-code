#pragma once

#include "artcode.hpp"
#include <algorithm>
#include <cstdio>
#include <fcntl.h>
#include <glm/glm.hpp>
#include <iostream>
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
    size_t size;
    Vec2   element[999];
};

struct Indx {
    size_t size;
    u32    element[9999];
};

// TODO:create shared memory for api and app
namespace Shared {
    struct Instance {
        Vert vertex;
        Indx index;
    };

    struct Region {
        size_t           size = 0;
        Shared::Instance instance[500];
    };

    struct Memory {
        static inline Shared::Region* region;

        static void load_shared_memory() {
            // uses POSIX functions
            int fd = shm_open("/artcode_instances", O_CREAT | O_RDWR, 0666);
            ftruncate(fd, sizeof(Shared::Region));

            region = (Shared::Region*)mmap(nullptr, sizeof(Shared::Region),
                                           PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

            close(fd);

            std::memset(region, 0, sizeof(Shared::Region));

            // clear on first init
            if (region->size < 0 || region->size >= 500)
                Shared::Memory::reset_instance();
        }

        static void cleanup() {
            munmap(region, sizeof(Shared::Region));
            shm_unlink("/artcode_instances");
        }

        // FIXME:this segfaults
        static void register_instance(const ArrayVec2& vertex, const ArrayU32 index) {
            // max num of instances
            if (region->size >= 500 || !region)
                return;

            // insert vertex
            for (const auto& vert : vertex) {
                region->instance->vertex.element[region->instance->vertex.size++] = vert;
            }
            // insert indices
            for (const auto& idx : index) {
                region->instance->index.element[region->instance->index.size++] = idx;
            }
            region->size++;
        }

        static Region get_instance() { return *region; }

        static size_t get_intance_size() { return region->size; }

        static Vert get_vertex(size_t idx) { return region->instance[idx].vertex; }

        static Indx get_index(size_t idx) { return region->instance[idx].index; }

        static void reset_instance() {
            region->size = 0;
            std::fill(std::begin(region->instance), std::end(region->instance),
                      Shared::Instance{});
        }
    };
} // namespace Shared
