// Example and test on how to use the Artcode API, subject to change
#include "components/comp.hpp"
#include <artcode.hpp>

int main() {
    // quad i.e a square
    Art::Quad qd;
    qd.w        = 100;
    qd.l        = 100;
    qd.position = {300, 300};
    qd.color    = "#322322";

    // skewed quad example i.e rectangle, skewPos is a std::array
    Art::Quad rc;
    rc.w        = 300;
    rc.l        = 100;
    rc.fill     = true;
    rc.position = {400, 300};
    rc.skew     = true;
    rc.skewPos  = {{{Vec2{100, 200}, 2}, {Vec2{-200, 400}, 4}, {Vec2{200, 0}, 1}}};
    rc.color    = "#696969";

    // circle with opacity, it ranges from 0.0 to 1.0
    // fill is false by default, no need to add or configure
    Art::Circle cr;
    cr.radius   = 100;
    cr.position = {500, 500};
    cr.stroke   = 5.0f;
    cr.opacity  = 0.5f;
    cr.color    = "#184184";

    // rotate triangle, rotate == degrees
    Art::Triangle tr;
    tr.base     = 100;
    tr.height   = 300;
    tr.position = {600, 300};
    tr.fill     = true;
    tr.rotate   = 30;
    tr.color    = "#454647";

    // pen tool exmaple where handle is enabled and have a vec2 position
    Art::Pen pn;
    pn.posiions = {
        {Vec2{100, 100}, {true, Vec2{100, 500}}}, {Vec2{400, 500}}, {Vec2{100, 500}}};
    pn.fill  = true;
    pn.color = "#7FD644";

    // draw function is called alaways at the end of all shape instances
    // the draw function is responsible for taking displaying all instances to artboard
    Art::Draw();

    return 0;
};
