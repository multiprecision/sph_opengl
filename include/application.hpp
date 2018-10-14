// Copyright (c) 2017-2018, Samuel Ivan Gunadi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
#include <atomic>
#include <mutex>
#include <vector>

// constants
#define SPH_NUM_PARTICLES 20000

#define SPH_PARTICLE_RADIUS 0.005f

#define SPH_WORK_GROUP_SIZE 128
// work group count is the ceiling of particle count divided by work group size
#define SPH_NUM_WORK_GROUPS ((SPH_NUM_PARTICLES + SPH_WORK_GROUP_SIZE - 1) / SPH_WORK_GROUP_SIZE)

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
    void run_simulation();
    void render();

    GLFWwindow* window = nullptr;
    uint64_t window_height = 1000;
    uint64_t window_length = 1000;

    std::atomic_uint64_t frame_number = 1;

    bool paused = false;

    int64_t scene_id = 0;

    // opengl
    uint32_t particle_position_vao_handle = 0;
    uint32_t render_program_handle = 0;
    uint32_t compute_program_handle[3] {0, 0, 0};
    uint32_t packed_particles_buffer_handle = 0;
};

} // namespace sph