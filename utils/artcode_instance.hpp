#pragma once

#include "artcode.hpp"
#include <cstdio>
#include <fcntl.h>
#include <glm/glm.hpp>
#include <stdexcept>
// NOTE:for now this projects only works for windows and linux
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

// NOTE:this header uses the artcode typedef such as Vec2, ArrayT and such,
// to avoid confusion and for the sake of consistency, only this header file uses the
// artcode typedef and nothing else// hides padding as this is for gpu alignmnet
struct SkewPos {
    Vec2 pos;
    int  index;

    SkewPos()
        : pos{},
          index(0),
          _padding(0) {}
    SkewPos(int i, Vec2 p)
        : pos{p},
          index{i},
          _padding{0} {}

  private:
    int _padding;
};

struct PHandle {
    Vec2 position;
    int  id;
    int  _padding;

    PHandle()
        : position(0.0f, 0.0f),
          id(0),
          _padding(0) {}
};

struct PenInstance {
    size_t               size;
    ArrayT<PHandle, 500> items;
};

struct SkewData {
    std::array<Vec2, 8>    skew_mesh;
    std::array<SkewPos, 8> skew_pos;
};

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

#ifdef _WIN32
    inline static HANDLE hmap_file = nullptr;
    inline static void*  ptr       = nullptr;
#endif

    struct Memory {
        static inline Shared::Region* region;

        // TODO:add exceptions
        static void load_shared_memory() {
            bool       first_init  = false;
            const auto region_size = sizeof(Shared::Region);
#ifdef _WIN32
            // open shared memory first
            hmap_file =
                OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "Local\\artcode_instances");

            if (hmap_file == NULL) {
                // create if not exist yet
                hmap_file =
                    CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                                       region_size, "Local\\artcode_instances");

                if (hmap_file != NULL) {
                    ptr = MapViewOfFile(hmap_file, FILE_MAP_ALL_ACCESS, 0, 0, region_size);

                    if (ptr == NULL) {
                        CloseHandle(hmap_file);
                        throw std::runtime_error(
                            "Failed to create and read shared memory!");
                    }
                } else {
                    throw std::runtime_error("Failed to create shared memory!");
                }
                // set region
                region = static_cast<Shared::Region*>(ptr);

                first_init = true;
            } else {
                ptr = MapViewOfFile(hmap_file, FILE_MAP_ALL_ACCESS, 0, 0, region_size);

                if (ptr == NULL) {
                    CloseHandle(hmap_file);
                    throw std::runtime_error("Failed to read shared memory!");
                }
                // set region
                region = static_cast<Shared::Region*>(ptr);
            }
#else
            // uses POSIX functions, returns -1 cuz of O_EXCL if process exists
            int fd     = shm_open("/artcode_instances", O_CREAT | O_EXCL | O_RDWR, 0666);
            first_init = fd != -1;
            // immidiate close if fd already exist
            if (!first_init) {
                fd = shm_open("/artcode_instances", O_RDWR, 0666);
            } else {
                // TODO:add proper error handling
                const auto result = ftruncate(fd, region_size);
                if (result == -1) {
                    assert("truncate failed!");
                    return;
                }
            }
            region = (Shared::Region*)mmap(nullptr, region_size, PROT_READ | PROT_WRITE,
                                           MAP_SHARED, fd, 0);

            close(fd);
#endif
            if (first_init)
                Shared::Memory::reset_instance();
        }

        // TODO:currently this function only runs if the program "safely" exits, this
        // means if program crashes, undefined behaviour causing a crash this function
        // doesnt get executed, find a way to execute this no matter what happen!
        static void cleanup() {
#ifdef _WIN32
            UnmapViewOfFile(ptr);
            CloseHandle(hmap_file);
#else
            munmap(region, sizeof(Shared::Region));
            shm_unlink("/artcode_instances");
#endif
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

        static void register_pen_data(const ArrayT<PenHandles, 500>& pen_handles) {
            check_instance_size();

            auto& inst = region->instance[region->size];

            size_t i = 0;
            // flatten the pen_handles array
            while (i < pen_handles.size()) {
                if (pen_handles[i].handles.handle) {
                    // id = 0 indicates that this position starts for a curve
                    inst.pen_data.items[inst.pen_data.size].position =
                        pen_handles[i].position;
                    inst.pen_data.items[inst.pen_data.size].id = 0;

                    // increment to insert to the next item
                    inst.pen_data.size += 1;
                    // this is the handle
                    inst.pen_data.items[inst.pen_data.size].position =
                        pen_handles[i].handles.handlePosition;
                    inst.pen_data.items[inst.pen_data.size].id = 2;
                } else {
                    inst.pen_data.items[inst.pen_data.size].position =
                        pen_handles[i].position;
                    // id = 1 indicates that this position starts and ends in segment
                    inst.pen_data.items[inst.pen_data.size].id = 1;
                }
                i++;
                // immidiate exit
                if (i == pen_handles.size())
                    break;

                inst.pen_data.size++;
            }
        }

        static void increment_instance_size() { region->size++; }

        // TODO:add proper error handling
        static void check_instance_size() {
            if (!region || region->size > 500) {
                throw std::runtime_error(
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
