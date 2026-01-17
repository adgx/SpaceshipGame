#pragma once

#ifdef _MSC_VER 
    #define FMT_UNICODE 0
#endif

#include <spdlog/spdlog.h>
#include <glad/gl.h>

#define DEFAULT_LOGGER_NAME "SpaceEngineLogger"
#ifdef SPACE_ENGINE_PLATFORM_WINDOWS
#define SPACE_ENGINE_BREAK __debugbreak();
#else 
#define SPACE_ENGINE_BREAK __builtin_trap();
#endif
#ifdef SPACE_ENGINE_DEBUG_CONFIG
#define SPACE_ENGINE_TRACE(...)\
            if(spdlog::get(DEFAULT_LOGGER_NAME) != nullptr)\
            {\
                spdlog::get(DEFAULT_LOGGER_NAME)->trace(__VA_ARGS__);\
            }
#define SPACE_ENGINE_INFO(...)\
            if(spdlog::get(DEFAULT_LOGGER_NAME) != nullptr)\
            {\
                spdlog::get(DEFAULT_LOGGER_NAME)->info(__VA_ARGS__);\
            }
#define SPACE_ENGINE_ERROR(...)\
            if(spdlog::get(DEFAULT_LOGGER_NAME) != nullptr)\
            {\
                spdlog::get(DEFAULT_LOGGER_NAME)->error(__VA_ARGS__);\
            }
#define SPACE_ENGINE_WARN(...)\
            if(spdlog::get(DEFAULT_LOGGER_NAME) != nullptr)\
            {\
                spdlog::get(DEFAULT_LOGGER_NAME)->warn(__VA_ARGS__);\
            }
#define SPACE_ENGINE_DEBUG(...)\
            if(spdlog::get(DEFAULT_LOGGER_NAME) != nullptr)\
            {\
                spdlog::get(DEFAULT_LOGGER_NAME)->debug(__VA_ARGS__);\
            }
#define SPACE_ENGINE_FATAL(...)\
            if(spdlog::get(DEFAULT_LOGGER_NAME) != nullptr)\
            {\
                spdlog::get(DEFAULT_LOGGER_NAME)->critical(__VA_ARGS__);\
            }
#define SPACE_ENGINE_ASSERT(x, msg)\
        if ((x))\
        {}\
        else\
        {\
            SPACE_ENGINE_FATAL("Assert: {}\n\tin file: {}\n\nton line: {}", #x, msg, __FILE__, __LINE__);\
            SPACE_ENGINE_BREAK;\
        }
#elif defined(SPACE_ENGINE_RELEASE_CONFIG)
        #define SPACE_ENGINE_TRACE(...)\
            if(spdlog::get(DEFAULT_LOGGER_NAME) != nullptr)\
            {\
                spdlog::get(DEFAULT_LOGGER_NAME)->trace(__VA_ARGS__);\
            }
#define SPACE_ENGINE_INFO(...)\
            if(spdlog::get(DEFAULT_LOGGER_NAME) != nullptr)\
            {\
                spdlog::get(DEFAULT_LOGGER_NAME)->info(__VA_ARGS__);\
            }
#define SPACE_ENGINE_ERROR(...)\
            if(spdlog::get(DEFAULT_LOGGER_NAME) != nullptr)\
            {\
                spdlog::get(DEFAULT_LOGGER_NAME)->error(__VA_ARGS__);\
            }
#define SPACE_ENGINE_WARN(...)\
            if(spdlog::get(DEFAULT_LOGGER_NAME) != nullptr)\
            {\
                spdlog::get(DEFAULT_LOGGER_NAME)->warn(__VA_ARGS__);\
            }
#define SPACE_ENGINE_DEBUG(...)\
            if(spdlog::get(DEFAULT_LOGGER_NAME) != nullptr)\
            {\
                spdlog::get(DEFAULT_LOGGER_NAME)->debug(__VA_ARGS__);\
            }
#define SPACE_ENGINE_FATAL(...)\
            if(spdlog::get(DEFAULT_LOGGER_NAME) != nullptr)\
            {\
                spdlog::get(DEFAULT_LOGGER_NAME)->critical(__VA_ARGS__);\
            }
#define SPACE_ENGINE_ASSERT(...) (void)0
#else
#define SPACE_ENGINE_TRACE(...) (void)0
#define SPACE_ENGINE_DEBUG(...) (void)0
#define SPACE_ENGINE_WARN(...) (void)0
#define SPACE_ENGINE_ERROR(...) (void)0
#define SPACE_ENGINE_INFO(...) (void)0
#define SPACE_ENGINE_FATAL(...) (void)0
#endif

static const char* getGLErrorString(GLenum error) 
{
    switch (error) 
    {
        case GL_NO_ERROR:
            return "No error";
        case GL_INVALID_ENUM:
            return "Invalid enum";
        case GL_INVALID_VALUE:
            return "Invalid value";
        case GL_INVALID_OPERATION:
            return "Invalid operation";
        case GL_OUT_OF_MEMORY:
            return "Out of memory";
        default:
            return "Unknown GL error";
    }
}

#define GL_CHECK(call)                                                   \
    do {                                                                 \
        call;                                                            \
        if (GLenum err = glGetError(); err != GL_NO_ERROR)               \
            SPACE_ENGINE_FATAL("GL error: %s for " #call, getGLErrorString(err)); \
    } while (0)

#define GL_CHECK_ERRORS()                                     \
    do {                                                      \
        if (GLenum err = glGetError(); err != GL_NO_ERROR)    \
            SPACE_ENGINE_FATAL("GL error: {}, file:{}, line:{}", getGLErrorString(err), __FILE__, __LINE__); \
    } while (0)

#define FT_CHECK(call)                                                  \
    do                                                                  \
    {                                                                   \
        if(int error = call; error)                                     \
        {                                                               \
            SPACE_ENGINE_FATAL("FreeType error: {} for " #call, error); \
            exit(-1);                                                   \
        }                                                               \
    }while(0)  