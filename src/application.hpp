#pragma once

#include "window.hpp"
namespace game 
{
    class Application
    {
        public:
            Application();
            ~Application();
            void run();

        private:
            Window m_window;
    };
}