#include "components/comp.hpp"
#include <artcode.hpp>
#include <iostream>

int main() {
    Component comp;

    std::cout << comp.x << std::endl;

    // quad example specifically square
    Art::Quad qd;
    qd.w        = 100;
    qd.l        = 100;
    qd.position = {300, 300};
    qd.color    = "#322322";

    Art::Draw();

    return 0;
};
