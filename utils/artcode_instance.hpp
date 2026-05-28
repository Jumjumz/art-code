#pragma once

#include "artcode.hpp"

struct ArtcodeInstance {
  public:
    static inline void set_vertices(const ArrayVec2& vertices) {
        ArtcodeInstance::vertices = vertices;
    }
    static inline ArrayVec2 get_vertices() { return ArtcodeInstance::vertices; }

    static inline void delete_vertices() { ArtcodeInstance::vertices.clear(); }

  private:
    static inline ArrayVec2 vertices;
};
