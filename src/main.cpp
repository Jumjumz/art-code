#include "core/app.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main() {
    Application art_code;

    try {
        art_code.run();
    } catch (const vk::SystemError &err) {
        std::cerr << "Vulkan Error: " << err.what() << std::endl;

        return EXIT_FAILURE;
    } catch (const std::exception &exc) {
        std::cerr << exc.what() << std::endl;

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
