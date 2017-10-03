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

// constants
#define SPH_PARTICLE_RADIUS 0.005f
#define SPH_TIME_STEP 0.0001f
#define SPH_PARTICLE_COUNT 20000

#define SPH_WORK_GROUP_SIZE 128
// work group count is the ceiling of particle count divided by work group size
#define SPH_GROUP_COUNT ((SPH_PARTICLE_COUNT + SPH_WORK_GROUP_SIZE - 1) / SPH_WORK_GROUP_SIZE)

namespace sph
{


class application
{
public:
    application();
    explicit application(int64_t scene_id);
    application(const application&) = delete;
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
    void step_forward();
    void render();

    GLFWwindow* window = nullptr;
    uint64_t window_height = 1000;
    uint64_t window_length = 1000;

    std::chrono::steady_clock::time_point frame_start;
    std::chrono::steady_clock::time_point frame_end;

    uint64_t frame_number = 1;
    double frame_time = 0;

    bool paused = false;

    int64_t scene_id = 0;

    // opengl
    uint32_t particle_position_vao_handle = 0;
    uint32_t render_program_handle = 0;
    uint32_t compute_program_handle[3] {0, 0, 0};
    uint32_t particle_buffer_handle = 0;

    struct particle_type
    {
        glm::vec2 position;
        glm::vec2 velocity;
        glm::vec2 force;
        float density;
        float pressure;
    };

};

} // namespace sph