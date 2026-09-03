#pragma once

#include "artcode.hpp"
#include <cstdio>
#include <fcntl.h>
#include <glm/glm.hpp>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <vulkan/vulkan_raii.hpp>

// this header uses the artcode typedef such as Vec2, ArrayT and such,
// to avoid confusion and for the sake of consistency, only this header file uses the
// artcode typedef and nothing else
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
    ArrayT<Vec4, 999> element;
};

struct Indx {
    size_t            size;
    ArrayT<u32, 9999> element;
};

struct SkewData {
    ArrayT<Vec2, 8>    skew_mesh;
    ArrayT<SkewPos, 8> skew_pos;
};

// TODO:shape data will be pass in push constants, except for shapes or intances that needed vertices and indices
struct PushConstants {
    Vec4  color;
    Vec2  pos;
    Vec2  center;
    Vec2  shape_data;
    Vec2  mesh_size;
    Vec2  p0;
    Vec2  p1;
    Vec2  p2;
    float stroke;
    float rotate;
    int   fill;
    int   skew;
    int   shape_type;
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

        // TODO:currently this function only runs if the program "safely" exits, this
        // means program crashes and other things this function doesnt get executed, find
        // a way to execute this no matter what happen!
        static void cleanup() {
            munmap(region, sizeof(Shared::Region));
            shm_unlink("/artcode_instances");
        }

        static void register_instance(const ArrayVec4& vertex, const ArrayU32& index,
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

        // TODO:complete this, for now get the constants first, missing skew data
        static void register_constants(const PushConstants& push_const,
                                       const SkewData&      skew_data) {
            if (!region || region->size > 500) {
                assert(
                    "Max instances reached! or something wrong with instance creation!");
                return;
            }

            auto& inst = region->instance[region->size];

            inst.constants = push_const;
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
