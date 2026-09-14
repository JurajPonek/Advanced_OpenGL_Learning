#pragma once

#include "camera.hpp"
#include "imgui.h"
#include "mouse_button_evet.hpp"
#include "Dscene.hpp"
#include "scene.hpp"
#include <windows.h>

namespace game
{
    class DebugUI
    {
        public:
            DebugUI(HWND window, Scene& scene, Camera& camera);
            ~DebugUI();
            void render() const;
            void add_mouse_event(const MouseButtonEvent& event) const;

        private:
            Scene& m_scene;
            Camera& m_camera;

    };
}