#include "scene.hpp"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_win32.h"
#include "color.hpp"
#include "log.hpp"
#include "opengl.hpp"
#include "window.hpp"
#include <functional>
#include <imgui.h>
#include <memory>
#include <string_view>


namespace game
{
    SceneManager::SceneManager() : m_scenes{}, m_current{nullptr}
    {
    }
    
    void SceneManager::reset()
    {
        ::glDisable(GL_STENCIL_TEST);
        ::glDisable(GL_DEPTH_TEST);
        ::glDisable(GL_BLEND);
        ::glDisable(GL_CULL_FACE);
        ::glUseProgram(0);
        ::glBindVertexArray(0);
        ::glBindFramebuffer(GL_FRAMEBUFFER, 0);
        ::glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        ::glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        ::glBindTexture(GL_TEXTURE_2D, 0);
        ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        ::glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    void SceneManager::render()
    {
        if (m_current)
        {
            m_current->on_render();
        }
        else
        {
            ::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            ::glClear(GL_COLOR_BUFFER_BIT);
        }
    }
    void SceneManager::update(float dt)
    {
        if (m_current)
        {
            m_current->on_update(dt);
        }
    }
    void SceneManager::render_ui()
    {
        ::ImGui_ImplOpenGL3_NewFrame();
        ::ImGui_ImplWin32_NewFrame();
        ::ImGui::NewFrame();
        ::ImGuiIO& io = ImGui::GetIO();
        ::ImGui::LabelText("FPS", "%0.1f", io.Framerate);
        if (m_current && ImGui::Button("Return"))
        {
            m_current->on_detach();
            m_current = nullptr;
            reset();
        }
        if (m_current)
        {
            m_current->on_imgui_render();
        }
        else
        {
            for (const auto& [name, scene] : m_scenes)
            {
                if (ImGui::Button(name.c_str()))
                {
                    m_current = scene();
                    m_current->on_attach();
                }
            }
        }
        ::ImGui::Render();
        ::ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void SceneManager::set_startup_scene(const std::function<std::unique_ptr<Scene>()>& scene)
    {
        m_current = scene();
        m_current->on_attach();
    }
} // namespace game