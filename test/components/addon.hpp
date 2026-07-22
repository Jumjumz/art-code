// this header file is an example on how the application adds header files to its sln file
// this will be seen in the test.rcd.template
#pragma once

#include <artcode.hpp>

// in the solution filem only the .cpp header file will be added, the reason is the
// "sources" key in the solution file ".rcd" takes all the files that the user added by
// clicking the "Include" button in the development panel will automatically add the files
// to the "sources" key to have a seamless build and compile of every fill added. Header
// files are not added in "sources" as header files are copied when doing and #include
// this means you can freely add .h or .hpp files as long as you #include the relative
// path the .cpp file
// currently this class does nothing, you can try to add instances to this one!
class AddOn {
    AddOn();

    void addon();
};
