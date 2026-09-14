#include "debug_ui.hpp"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_win32.h"
#include "matrix4.hpp"
#include "mouse_button_evet.hpp"
#include "ImGuizmo.h"
#include <cstring>
#include <format>
#include <ranges>

namespace game
{
    DebugUI::DebugUI(HWND window, Scene& scene, Camera& camera)
        :   m_scene{scene}, m_camera{camera}
    {
        ::IMGUI_CHECKVERSION();
        ::ImGui::CreateContext();
        ::ImGuiIO& io = ImGui::GetIO();
        ::ImGui::StyleColorsDark();
        ::ImGui_ImplWin32_InitForOpenGL(window);
        ::ImGui_ImplOpenGL3_Init();

    }
    DebugUI::~DebugUI()
    {
        ::ImGui_ImplOpenGL3_Shutdown();
        ::ImGui_ImplWin32_Shutdown();
        ::ImGui::DestroyContext();   
    }
    void DebugUI::render() const
    {
        auto& io = ::ImGui::GetIO();
        ::ImGui_ImplOpenGL3_NewFrame();
        ::ImGui_ImplWin32_NewFrame();
        ::ImGui::NewFrame();
        // ::ImGuizmo::SetOrthographic(false);
        // ::ImGuizmo::BeginFrame();
        // ::ImGuizmo::Enable(true);
        // ::ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        // static auto selected_point = 0;
        // if (ImGui::Button("Add light"))
        // {
        //     m_scene.points.push_back(m_scene.points.back());
        //     selected_point = m_scene.points.size() - 1u;
        // }
        // ::ImGui::LabelText("FPS", "%0.1f", io.Framerate);
        // if (::ImGui::CollapsingHeader("ambient"))
        // {
        //     float colors[3]{};
        //     std::memcpy(colors, &m_scene.ambient, sizeof(colors));
        //     if (::ImGui::ColorPicker3("ambient color", colors))
        //     {
        //         std::memcpy(&m_scene.ambient, colors, sizeof(colors));   
        //     }
        // }
        // if (::ImGui::CollapsingHeader("directional"))
        // {
        //     float colors[3]{};
        //     std::memcpy(colors, &m_scene.directional.color, sizeof(colors));
        //     if (::ImGui::ColorPicker3("directional color", colors))
        //     {
        //         std::memcpy(&m_scene.directional.color, colors, sizeof(colors));
        //     }
        // }
        // for (const auto& [index, point] : m_scene.points | std::views::enumerate)
        // {
        //     float colors[3]{};
        //     std::memcpy(colors, &point.color, sizeof(colors));
        //     const auto header_name= std::format("color {}", index);
        //     const auto picker_name = std::format("color {}", index);
        //     const auto const_name = std::format("constant {}", index);
        //     const auto linear_name = std::format("linear {}", index);
        //     const auto quad_name = std::format("quadratic {}", index);
        //     if (::ImGui::CollapsingHeader(header_name.c_str()))
        //     {
        //         if (::ImGui::ColorPicker3(picker_name.c_str(), colors))
        //         {
        //             point.color.r = colors[0];
        //             point.color.g = colors[1];
        //             point.color.b = colors[2];
        //             selected_point = index;
        //         }
        //         ::ImGui::SliderFloat(const_name.c_str(), &point.const_attenuation, 0.0f, 1.0f);
        //         ::ImGui::SliderFloat(linear_name.c_str(), &point.linear_attenuation, 0.0f, 1.0f);
        //         ::ImGui::SliderFloat(quad_name.c_str(), &point.quad_attenuation, 0.0f, .1f);
        //     }
        // }
        // auto& point = m_scene.points[selected_point];
        // auto translate = Matrix4{point.position};
        // ::ImGuizmo::Manipulate(m_camera.get_view().data(), m_camera.get_projection().data(), ::ImGuizmo::TRANSLATE,
        //                        ::ImGuizmo::WORLD, const_cast<float*>(translate.data().data()), nullptr, nullptr,
        //                        nullptr, nullptr);
        // point.position.x = translate.data()[12];
        // point.position.y = translate.data()[13];
        // point.position.z = translate.data()[14];
        ::ImGui::Render();
        ::ImGui_ImplOpenGL3_RenderDrawData(::ImGui::GetDrawData());

    }

    void DebugUI::add_mouse_event(const MouseButtonEvent& event) const
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(0, event.get_state() == MouseButtonState::DOWN);

    }

}

