#pragma once
#include "string_map.hpp"
#include "window.hpp"
#include <functional>
#include <gl/gl.h>
#include <imgui.h>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include "error.hpp"


namespace game
{
    class Scene
    {
      public:
        Scene() = default;
        virtual ~Scene() = default;
        virtual void on_render() = 0;
        virtual void on_update(float dt) = 0;
        virtual void on_imgui_render() = 0;
        virtual void on_attach() = 0;
        virtual void on_detach() = 0;
    };

    class SceneManager
    {
      public:
        SceneManager(const Window& window);
        ~SceneManager();
        template <typename T = Scene, typename... Args> void add_scene(std::string_view name, Args&&... args)
        {
            ensure(m_scenes.find(name) == std::ranges::cend(m_scenes), "This scene already exists");
            m_scenes.emplace(name, [... args = std::forward<Args>(args)]() mutable
                                  { return std::make_unique<T>(args...); });
        }
        void reset();
        void render();
        void update(float dt);
        void render_ui();
      private:
        StringMap<std::function<std::unique_ptr<Scene>()>> m_scenes;
        std::unique_ptr<Scene> m_current;
    };

} // namespace game