#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace ne
{
    class Window
    {
    public:
        Window(int w, int h, std::string name);
        ~Window();

		// Remove copy constructor
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

        bool shouldClose() { return glfwWindowShouldClose(window); }
        VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
        bool wasWindowResized() { return frameBufferResized; }
        void resetWindowResizedFlag() { frameBufferResized = false; }
        GLFWwindow* getGLFWwindow() const { return window; }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

    private:
        static void frameBufferResizedCallback(GLFWwindow* window, int width, int height);
        void initWindow();

        int width;
        int height;
        bool frameBufferResized;

        std::string windowName;
        GLFWwindow *window;
    };
}
