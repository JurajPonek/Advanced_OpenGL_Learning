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
        virtual void on_render() {};
        virtual void on_update(float dt) {};
        virtual void on_imgui_render() {};
        virtual void on_attach() {};
        virtual void on_detach() {};
    };

    class SceneManager
    {
      public:
        SceneManager();
        template <typename T, typename... Args> void add_scene(std::string_view name, Args&&... args)
        {
            ensure(m_scenes.find(name) == std::ranges::cend(m_scenes), "This scene already exists");
            m_scenes.emplace(name, [... args = std::forward<Args>(args)]() mutable
                                  { return std::make_unique<T>(args...); });
        }
        void reset();
        void render();
        void update(float dt);
        void render_ui();
        void set_startup_scene(const std::function<std::unique_ptr<Scene>()>& scene);
      private:
        StringMap<std::function<std::unique_ptr<Scene>()>> m_scenes;
        std::unique_ptr<Scene> m_current;
    };

} // namespace game