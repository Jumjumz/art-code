#include "comp.hpp"

Component::Component() {};

// definition of draw bolt function
void Component::rightTriangles() {
    // flipped triangle in x axis
    rt1.base     = 100;
    rt1.height   = 300;
    rt1.type     = TriangleTypes::Right;
    rt1.position = {700, 400};
    rt1.fill     = true;
    rt1.rotate   = 180;
    rt1.color    = "#C343233";

    // normal right triangle
    rt2.base     = 100;
    rt2.height   = 300;
    rt2.type     = TriangleTypes::Right;
    rt2.position = {700, 400};
    rt2.fill     = true;
    rt2.color    = "#C343233";
};
