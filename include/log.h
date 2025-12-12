#pragma once

#ifdef _MSC_VER 
    #define FMT_UNICODE 0
#endif

#include <spdlog/spdlog.h>



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