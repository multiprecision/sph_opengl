/*
 * Copyright (c) 2017, Samuel I. Gunadi
 * All rights reserved.
 */

#pragma once

#define GLEW_STATIC
#include <gl/glew.h>
#include <glfw/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <chrono>
#include <cstdint>
#include <string>

namespace sph
{
//constants
const float particle_length = 0.005f;
const float time_step = 0.0001f;
const size_t particle_count = 20000;

const size_t work_group_size = 128;
const uint32_t group_count = (size_t)std::ceil((float)particle_count / work_group_size);

class application
{
public:
    application();
    ~application();
    void run();

private:
    void initialize_window();
    void initialize_opengl();
    void destroy_window();
    void destroy_opengl();
    GLuint compile_shader(std::string path_to_file, GLenum shader_type);
    void check_program_linked(GLuint shader_program_handle);
    void main_loop();
    void invoke_compute_shader();
    void render();

    GLFWwindow* window = nullptr;
    uint64_t window_height = 1000;
    uint64_t window_length = 1000;

    std::chrono::steady_clock::time_point frame_start;
    std::chrono::steady_clock::time_point cpu_end;
    std::chrono::steady_clock::time_point frame_end;

    uint64_t frame_number = 0;
    float frame_time = 0;
    float cpu_time = 0;
    float gpu_time = 0;

    // opengl
    uint32_t particle_position_vao_handle = 0;
    uint32_t render_program_handle = 0;
    uint32_t compute_program_handle[3] = {0, 0, 0};
    uint32_t particle_buffer_handle = 0;

    struct particle_t
    {
        glm::vec2 position;
        glm::vec2 velocity;
        glm::vec2 force;
        float density;
        float pressure;
    };

};

} // namespace sph