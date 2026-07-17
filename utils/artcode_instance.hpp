#pragma once

#include "artcode.hpp"
#include <cstdio>
#include <fcntl.h>
#include <glm/glm.hpp>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <vulkan/vulkan_raii.hpp>

struct Vertex {
    Vec2 pos;

    static vk::VertexInputBindingDescription get_binding_description() {
        return {0, sizeof(Vertex), vk::VertexInputRate::eVertex};
    };

    static ArrayT<vk::VertexInputAttributeDescription, 1> get_attribute_description() {
        return {
            vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32Sfloat,
                                                offsetof(Vertex, pos)},
        };
    };
};

struct Vert {
    size_t            size;
    ArrayT<Vec2, 999> element;
};

struct Indx {
    size_t            size;
    ArrayT<u32, 9999> element;
};

struct SkewData {
    ArrayT<Vec2, 8>    skew_mesh;
    ArrayT<SkewPos, 8> skew_pos;
};

struct PushConstants {
    Vec4  color;
    Vec2  center;
    float stroke;
    float rotate;
    int   fill;
    int   skew;
};

namespace Shared {
    struct Instance {
        Vert          vertex;
        Indx          index;
        PushConstants constants;
        SkewData      skew_data;
    };

    struct Region {
        size_t                        size = 0;
        ArrayT<Shared::Instance, 500> instance;
    };

    struct Memory {
        static inline Shared::Region* region;

        static void load_shared_memory() {
            // uses POSIX functions, returns -1 cuz of O_EXCL if process exists
            int fd = shm_open("/artcode_instances", O_CREAT | O_EXCL | O_RDWR, 0666);
            const bool first_init = fd != -1;
            // immidiate close if fd already exist
            if (!first_init) {
                fd = shm_open("/artcode_instances", O_RDWR, 0666);
            } else {
                const auto& result = ftruncate(fd, sizeof(Shared::Region));
                if (result == -1) {
                    std::cerr << "truncate failed!" << std::endl;
                    return;
                }
            }
            region = (Shared::Region*)mmap(nullptr, sizeof(Shared::Region),
                                           PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

            close(fd);

            if (first_init)
                Shared::Memory::reset_instance();
        }

        static void cleanup() {
            munmap(region, sizeof(Shared::Region));
            shm_unlink("/artcode_instances");
        }

        static void register_instance(const ArrayVec2& vertex, const ArrayU32& index,
                                      const PushConstants& push_constants,
                                      const SkewData&      skew_data) {
            if (region->size > 500 || !region)
                return;

            auto& inst = region->instance[region->size];
            // insert vertex
            for (const auto& vert : vertex) {
                inst.vertex.element[inst.vertex.size++] = vert;
            }
            // insert indices
            for (const auto& idx : index) {
                inst.index.element[inst.index.size++] = idx;
            }
            inst.constants = push_constants;
            inst.skew_data = skew_data;
            region->size++;
        }

        static size_t get_intance_size() { return region->size; }

        static Shared::Instance get_instance(size_t idx) { return region->instance[idx]; }

        static PushConstants get_constants(size_t idx) {
            return region->instance[idx].constants;
        }

        static SkewData get_skew_data(size_t idx) {
            return region->instance[idx].skew_data;
        }

        static void reset_instance() {
            region->size = 0;
            std::fill(region->instance.begin(), region->instance.end(), Shared::Instance{});
        }
    };
} // namespace Shared
