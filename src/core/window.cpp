#include "window.h"


// TODO: replace runtime errors with something better
#include <stdexcept>

namespace ne
{
    Window::Window(int w, int h, std::string name) : 
        width(w), 
        height(h), 
        windowName(name),
        frameBufferResized(false)
    {
        initWindow();
    }

    Window::~Window()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

	void Window::frameBufferResizedCallback(GLFWwindow* window, int width, int height)
	{
        Window* veWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        veWindow->frameBufferResized = true;
        veWindow->width = width;
        veWindow->height = height;
	}

	void Window::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, frameBufferResizedCallback);
    }

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
    {
        if(glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
            throw std::runtime_error("failed to create window surface");
    }
}
