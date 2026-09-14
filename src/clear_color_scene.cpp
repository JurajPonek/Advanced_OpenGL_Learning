#include "clear_color_scene.hpp"
#include "opengl.hpp"
#include <imgui.h>

namespace game
{
    ClearColorScene::ClearColorScene()
        :m_color{}
    {

    };
    void ClearColorScene::on_render()
    {
        ::glClearColor(m_color.r, m_color.g, m_color.b, 1.0f);
        ::glClear(GL_COLOR_BUFFER_BIT);
    };
    void ClearColorScene::on_imgui_render()
    {
        ImGui::ColorPicker3("Background Color", reinterpret_cast<float*>(&m_color));
    };
}