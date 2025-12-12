#include "windowManager.h"
#include "log.h"

namespace SpaceEngine
{
    void window_maximize_callback(GLFWwindow* window, int maximized);
    void window_resize_callback(GLFWwindow* window, int width, int height);

    int WindowManager::height = SPACE_ENGINE_MIN_RES_H, WindowManager::width = SPACE_ENGINE_MIN_RES_W;
    Matrix4 WindowManager::sceenProjMatrix = glm::ortho(0.0f,(float)WindowManager::width,(float)WindowManager::height,0.0f);
    int WindowManager::xpos = 0, WindowManager::ypos = 0;
    GLFWwindow* WindowManager::window = nullptr;
    bool WindowManager::fullScreenState = false;
    GLFWmonitor* WindowManager::monitor = nullptr;

    void WindowManager::Initialize()
    {
        setUpGLFW();
    }

    int WindowManager::WindowShouldClose()
    {
        return glfwWindowShouldClose(window);
    }

    void WindowManager::SetWindowShouldClose()
    {
        glfwSetWindowShouldClose(window, true);
    }
    
    void WindowManager::PollEvents()
    {
        glfwPollEvents();
    }
    void WindowManager::Windowed()
    {
        if(fullScreenState)
        {
            fullScreenState = false;
            glfwSetWindowMonitor(WindowManager::window,
		                        nullptr,
		                        WindowManager::xpos,
		                        WindowManager::ypos,
		                        SPACE_ENGINE_MIN_RES_W,
                                SPACE_ENGINE_MIN_RES_H,
		                        0);
            //glViewport(0, 0, SPACE_ENGINE_MIN_RES_W, SPACE_ENGINE_MIN_RES_H);
        }
    }

    void WindowManager::SwapBuffers()
    {
        glfwSwapBuffers(window);
    }
    bool WindowManager::setUpGLFW()
    {
        SPACE_ENGINE_TRACE("App - set up GLFW");
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width, height, "Spaceship", NULL, NULL);
        
        //setup window
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwSetWindowSizeLimits(window, SPACE_ENGINE_MIN_RES_W, SPACE_ENGINE_MIN_RES_H, GLFW_DONT_CARE, GLFW_DONT_CARE);
        glfwSetWindowAspectRatio(window, 16, 9);
        glfwSetWindowMaximizeCallback(window, window_maximize_callback);

        //saving the position where is created
        glfwGetWindowPos(window, &xpos, &ypos);

        if(window == nullptr)
        {
            SPACE_ENGINE_ERROR("Failed to create GLFW window");
            return false;
        }

        glfwMakeContextCurrent(window);

        int version = gladLoadGL(glfwGetProcAddress);

        if(version == 0)
        {
            SPACE_ENGINE_ERROR("Failed to initilize OpenGL Context");
            return false;
        }
        //set the OpenGL depth buffer
        glEnable(GL_DEPTH_TEST);
        //set the OpenGL framebuffer
        //glViewport(0, 0, SPACE_ENGINE_MIN_RES_W, SPACE_ENGINE_MIN_RES_H); 
        //callbacks
        glfwSetFramebufferSizeCallback(window, window_resize_callback);

        SPACE_ENGINE_INFO("WindowManager - GLFW setup done");
        return true;
    }

    void WindowManager::Shutdown()
    {
        glfwTerminate();
    }

    GLFWmonitor* get_window_monitor(GLFWwindow* window)
    {
        int wx, wy, ww, wh;
        glfwGetWindowPos(window, &wx, &wy);
        glfwGetWindowSize(window, &ww, &wh);

        int monitorCount;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
        GLFWmonitor* bestMonitor = glfwGetPrimaryMonitor();
        
        if(monitorCount == 1)
            return bestMonitor;

        int bestOverlap = 0;

        for (int i = 0; i < monitorCount; i++) {
            int mx, my, mw, mh;
            glfwGetMonitorWorkarea(monitors[i], &mx, &my, &mw, &mh);

            // Compute overlap area
            int overlapW = std::max(0, std::min(wx + ww, mx + mw) - std::max(wx, mx));
            int overlapH = std::max(0, std::min(wy + wh, my + mh) - std::max(wy, my));
            int overlap = overlapW * overlapH;

            if (overlap > bestOverlap) {
                bestOverlap = overlap;
                bestMonitor = monitors[i];
            }
        }

        return bestMonitor;
    }

    void window_resize_callback(GLFWwindow* window, int width, int height)
    {
        SPACE_ENGINE_DEBUG("Resize frame buffer w{} h{}", width, height);
        if((width > 1900 && width < 1940) && (height>980 && height<1040))
        {
            width = 1920;
            height = 1080;
        }
        glViewport(0, 0, width, height);
    }

    //callbacks
    void window_maximize_callback(GLFWwindow* window, int maximized)
    {
        if (maximized)
        {
            // The window was maximized
            WindowManager::fullScreenState = true;
            WindowManager::monitor = get_window_monitor(window);
            const GLFWvidmode* mode = glfwGetVideoMode(WindowManager::monitor);
            
            glfwSetWindowMonitor(window, WindowManager::monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            //glViewport(0, 0, SPACE_ENGINE_MAX_RES_W, SPACE_ENGINE_MAX_RES_H);
        }

    }

}
