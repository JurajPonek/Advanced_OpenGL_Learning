add_executable(game main.cpp
                    window.cpp
                    shader.cpp
                    material.cpp
                    mesh.cpp
                    renderer.cpp
                    exception.cpp
                    camera.cpp
                    entity.cpp
                    key_event.cpp
                    mouse_event.cpp
                    bufffer.cpp
                    resource_loader.cpp
                    texture.cpp
                    sampler.cpp
                    debug_ui.cpp
                    mouse_button_evet.cpp
                    mesh_loader.cpp
                    
)
target_include_directories(game PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(game PRIVATE opengl32)
target_link_libraries(game PRIVATE 
    stdc++exp 
    backtrace
)
target_compile_features(game PUBLIC cxx_std_26)
target_compile_options(game PUBLIC 
    -Wall 
    -Werror 
    -Wno-unused-variable 
    -Wno-unused-function
    $<$<CONFIG:Debug>:-g>
)
target_compile_definitions(game PRIVATE
    WIN32_LEAN_AND_MEAN     
    # NOMINMAX              
)
