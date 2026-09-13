cmake_minimum_required(VERSION 3.22)
project(game
        VERSION "0.0.1"
        LANGUAGES C CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
add_compile_options(-Wno-unknown-pragmas)


include(FetchContent)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master      
    GIT_SHALLOW    TRUE        
)
FetchContent_MakeAvailable(stb)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        master      
    GIT_SHALLOW    TRUE    
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp  
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC dwmapi)

FetchContent_Declare(
    imguizmo
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imguizmo)
target_link_libraries(imguizmo PUBLIC imgui)
target_include_directories(imguizmo PUBLIC ${imguizmo_SOURCE_DIR})

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)              
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)    
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)                
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)               

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)             
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)           
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)      
set(ASSIMP_BUILD_DOCS OFF CACHE BOOL "" FORCE)              

set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)

set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)

set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)       # .fbx (animácie, modely z Blenderu/Maya)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)       # .obj (klasický statický formát)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)      # .gltf, .glb (moderný štandard pre PBR modely)

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v5.4.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(assimp)

add_subdirectory(src)

target_link_libraries(game PRIVATE stb imgui imguizmo assimp)