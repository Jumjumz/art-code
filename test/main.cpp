#include "components/comp.hpp"
#include <artcode.hpp>
#include <iostream>

int main() {
    Component comp;

    std::cout << comp.x << std::endl;

    // quad example i.e square
    Art::Quad qd;
    qd.w        = 100;
    qd.l        = 100;
    qd.position = {300, 300};
    qd.color    = "#322322";

    // skewed quad example i.e rectangle
    Art::Quad rc;
    rc.w        = 300;
    rc.l        = 100;
    rc.fill     = true;
    rc.position = {400, 300};
    rc.skew     = true;
    rc.skewPos  = {{{Vec2{100, 200}, 2}, {Vec2{-200, 400}, 4}}};
    rc.color    = "#696969";

    Art::Draw();

    return 0;
};
