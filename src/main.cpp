#include <iostream>

#include "../include/Application.hpp"

#include <vulkan/vulkan.h>

int main(int argc, char* argv[]) {
    Application* app = new Application();
    app->run();

    return 0;
}
