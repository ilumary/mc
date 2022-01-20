#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <utility>

struct GLFWwindow;

class Window {
public:
    Window() = default;
    Window(int width, int height, const char* title);

    ~Window();

    void swap_buffers() noexcept;

    bool should_close();

    void pull_events();

    inline GLFWwindow* glfw_window() { return window_; }

private:
    GLFWwindow* window_ = nullptr;
};

#endif // WINDOW_HPP