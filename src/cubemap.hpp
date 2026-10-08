#pragma once


#include "auto_release.hpp"
#include "resource_loader.hpp"
#include <string>
#include <vector>
#include "opengl.hpp"

namespace game
{
    class CubeMap
    {
        public:
            CubeMap(const std::vector<std::string>& faces, const ResourceLoader& loader);
            CubeMap(const std::string& hrd_map_name, const ResourceLoader& loader);
            ::GLuint get_native_handle() const;


        private:
            AutoRelease<::GLuint> m_handle;
    };
}