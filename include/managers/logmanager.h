#pragma once

namespace SpaceEngine::Managers
{
    class LogManager
    {
        public:
            LogManager() = default;
            ~LogManager() = default;

            void Initialize();
            void Shutdown();
    };
}