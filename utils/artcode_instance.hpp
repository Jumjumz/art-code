#pragma once

#include "artcode.hpp"
#include <algorithm>

struct ArtcodeInstance {
  public:
    static inline void register_instance(detail::IPen* instance) {
        ArtcodeInstance::instance.push_back(instance);
    }

    static inline std::vector<detail::IPen*> get_instance() {
        return ArtcodeInstance::instance;
    }

    static inline void delete_instance(detail::IPen* instance) {
        // iterator
        const auto it = std::remove(ArtcodeInstance::instance.begin(),
                                    ArtcodeInstance::instance.end(), instance);
        ArtcodeInstance::instance.erase(it, ArtcodeInstance::instance.end());
    }

  private:
    static inline std::vector<detail::IPen*> instance;
};
