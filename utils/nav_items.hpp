#pragma once

#include <string>
#include <utility>
#include <vector>

using NavMenuItems =
    const std::vector<std::pair<const std::string, const std::string>>;
using NavMenu = std::pair<const std::string, NavMenuItems>;

// All menu and menu items for main naviation
struct NavMainItems {
    static inline const std::vector<NavMenu> MENUS = {
        {"File", {{"New", "Ctrl+n"}, {"Open", "Ctrl+o"}}},
        {"Edit", {{"Artboard", "Ctrl+ar"}}},
        {"Window", {{"Panel", "Ctrl+p"}}},
        {"Help", {{"Documentation", "Ctrl+dd"}, {"About", ""}, {"Version", ""}}}};
};
