#include "Window.hpp"

#include <GLFW/glfw3.h>
#include <fmt/core.h>

Window::Window(int width, int height, const char* title) {
    if (!glfwInit()) { std::exit(1); }
    fmt::print("GLFW initialised\n");

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window_) { fmt::print("Cannot create a glfw window"); std::exit(1); }
}

Window::~Window() { 
    glfwDestroyWindow(window_); 
    glfwTerminate();
}

void Window::pull_events() { glfwPollEvents(); }

void Window::swap_buffers() noexcept { glfwSwapBuffers(window_); }

bool Window::should_close() { return glfwWindowShouldClose(window_); }