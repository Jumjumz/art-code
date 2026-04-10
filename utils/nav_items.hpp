#pragma once

#include <string>
#include <tuple>
#include <vector>

using NavMenuItems = const std::vector<std::tuple<std::string, std::string>>;
using NavMenu = std::tuple<const std::string, NavMenuItems>;

// All menu and menu items for main naviation
struct NavMainItems {
    static inline const std::vector<NavMenu> MENUS = {
        {"File", {{"New", "Ctrl+n"}, {"Open", "Ctrl+o"}}},
        {"Edit", {{"Artboard", "Ctrl+ar"}}},
        {"Window", {{"Panel", "Ctrl+p"}}},
        {"Help", {{"Documentation", "Ctrl+dd"}, {"About", ""}, {"Version", ""}}}};
};

struct NavBuildItems {
    static inline const NavMenuItems PANEL = {{"Files", "Ctrl+e"},
                                              {"Debug", "Ctrl+d"},
                                              {"Build", "Ctrl+b"},
                                              {"Run", "Ctrl+r"}};
};
