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
struct PHandle {
    Vec2 position;
    Vec2 uv;
    int  id;
    int  _padding;

    PHandle()
        : position(0.0f, 0.0f),
          uv(0.0f, 0.0f),
          id(0),
          _padding(0) {}
};

struct PenInstance {
    size_t               size;
    ArrayT<PHandle, 500> items;
};

struct SkewData {
    ArrayT<Vec2, 8>    skew_mesh;
    ArrayT<SkewPos, 8> skew_pos;
};

// TODO:shape data will be pass in push constants, except for shapes or intances that needed vertices and indices
struct PushConstants {
    Vec4  bg_color;
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
        PushConstants constants;
        PenInstance   pen_data;
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
                const auto result = ftruncate(fd, sizeof(Shared::Region));
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
        // means if program crashes, undefined behaviour causing a crash this function
        // doesnt get executed, find a way to execute this no matter what happen!
        static void cleanup() {
            munmap(region, sizeof(Shared::Region));
            shm_unlink("/artcode_instances");
        }

        static void register_constants(const PushConstants& push_const) {
            check_instance_size();

            auto& inst = region->instance[region->size];

            inst.constants = push_const;
        }

        static void register_skew_data(const SkewData& skew_data) {
            check_instance_size();

            auto& inst     = region->instance[region->size];
            inst.skew_data = skew_data;
        }

        static void register_pen_data(const VectorT<PenHandles>& pen_handles) {
            check_instance_size();

            auto& inst = region->instance[region->size];

            size_t i = 0;
            // flatten the pen_handles array
            while (i < pen_handles.size()) {
                // set to segment
                inst.pen_data.items[inst.pen_data.size].position = pen_handles[i].position;
                inst.pen_data.items[inst.pen_data.size].uv = Vec2{0.0f, 0.0f};
                // id = 1 indicates that this position starts for a segment
                inst.pen_data.items[inst.pen_data.size].id = 1;
                if (pen_handles[i].handles.handle) {
                    // id = 0 indicates that this position starts for a curve
                    inst.pen_data.items[inst.pen_data.size].id = 0;

                    // increment to insert to the next item
                    inst.pen_data.size += 1;
                    // this is the handle
                    inst.pen_data.items[inst.pen_data.size].position =
                        pen_handles[i].handles.handlePosition;
                    inst.pen_data.items[inst.pen_data.size].uv = Vec2{0.5f, 0.0f};
                    inst.pen_data.items[inst.pen_data.size].id = 2;
                } else {
                    inst.pen_data.items[inst.pen_data.size].position =
                        pen_handles[i].position;
                    inst.pen_data.items[inst.pen_data.size].uv = Vec2{1.0f, 1.0f};
                    // id = 1 indicates that this position is a segment
                    inst.pen_data.items[inst.pen_data.size].id = 1;
                }
                i++;
                inst.pen_data.size++;
            }
        }

        static void increment_instance_size() { region->size++; }

        static void check_instance_size() {
            if (!region || region->size > 500) {
                assert(
                    "Max instances reached! or something wrong with instance creation!");
                return;
            }
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
