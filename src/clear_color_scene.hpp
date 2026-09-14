#pragma once
#include "color.hpp"
#include "scene.hpp"

namespace game
{
    class ClearColorScene : public Scene
    {
      public:
        ClearColorScene();
        virtual void on_render() override;
        virtual void on_imgui_render() override;
      private:
        Color m_color;
    };
} // namespace game