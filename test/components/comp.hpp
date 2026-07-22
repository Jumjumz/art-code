#pragma once

#include <artcode.hpp>

// Test for using class where draw function is called in main function
class Component {
  public:
    // declare constructor
    Component();
    // The instances are declared as class members because instances are automatically
    // cleanedup if it reaches the end of the scope
    // it is safe to declare the instances as class members because classes resources gets
    // cleanup after the program ends
    Art::Triangle rt1;
    Art::Triangle rt2;

    // config of the triangles will be in here
    void rightTriangles();

  private:
};
