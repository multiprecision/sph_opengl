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

#include "application.hpp"

#include <cmath>
#include <string>
#include <algorithm>
#include <exception>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <thread>

#ifdef _DEBUG
void APIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* data)
{
    const char* source_str;
    const char* type_str;
    const char* severity_str;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        source_str = "SOURCE_API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        source_str = "SOURCE_WINDOW_SYSTEM";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        source_str = "SOURCE_SHADER_COMPILER";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        source_str = "SOURCE_THIRD_PARTY";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        source_str = "SOURCE_APPLICATION";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        source_str = "SOURCE_OTHER";
        break;
    default:
        source_str = "?";
        break;
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        type_str = "TYPE_ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        type_str = "TYPE_DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        type_str = "TYPE_UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        type_str = "TYPE_PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        type_str = "TYPE_PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_OTHER:
        type_str = "TYPE_OTHER";
        break;
    case GL_DEBUG_TYPE_MARKER:
        type_str = "TYPE_MARKER";
        break;
    default:
        type_str = "?";
        break;
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        severity_str = "SEVERITY_HIGH";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        severity_str = "SEVERITY_MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        severity_str = "SEVERITY_LOW";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        severity_str = "SEVERITY_NOTIFICATION";
        break;
    default:
        severity_str = "?";
        break;
    }
    std::cout << "[" << source_str << "][" << type_str << "][" << id << "][" << severity_str << "] " << msg << std::endl;
}
#endif

namespace sph
{

application::application()
{
    initialize_window();
    initialize_opengl();
}

application::application(int64_t scene_id)
{
    this->scene_id = scene_id;
    initialize_window();
    initialize_opengl();
}

application::~application()
{
    destroy_opengl();
    destroy_window();
}

void application::destroy_window()
{
    if (window != nullptr)
    {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

void application::destroy_opengl()
{
    glDeleteProgram(render_program_handle);
    glDeleteProgram(compute_program_handle[0]);
    glDeleteProgram(compute_program_handle[1]);
    glDeleteProgram(compute_program_handle[2]);

    glDeleteVertexArrays(1, &particle_position_vao_handle);
    glDeleteBuffers(1, &packed_particles_buffer_handle);

}

void application::run()
{
    // to measure performance
    std::thread(
        [this]()
        {
            std::this_thread::sleep_for(std::chrono::seconds(20));
            std::cout << "[INFO] frame count after 20 seconds: " << frame_number << std::endl;
        }
    ).detach();

    while (!glfwWindowShouldClose(window))
    {
        main_loop();
    }
}

void application::initialize_window()
{
    if (!glfwInit())
    {
        throw std::runtime_error("glfw initialization failed");
    }
#ifdef _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
    window = glfwCreateWindow(1000, 1000, "", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        throw std::runtime_error("window creation failed");
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // set vertical sync off
                            // pass Application pointer to the callback using GLFW user pointer
    glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
    // set key callback
    auto key_callback = [](GLFWwindow* window, int key, int scancode, int action, int mode)
    {
        auto app_ptr = reinterpret_cast<sph::application*>(glfwGetWindowUserPointer(window));
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            app_ptr->paused = !app_ptr->paused;
        }
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    };

    glfwSetKeyCallback(window, key_callback);
}

void application::initialize_opengl()
{
    if (gl3wInit()) {
        throw std::runtime_error("failed to initialize OpenGL");
    }
    if (!gl3wIsSupported(4, 6)) {
        throw std::runtime_error("OpenGL 4.6 is not supported");
    }
    // get version info 
    std::cout << "[INFO] OpenGL vendor: " << glGetString(GL_VENDOR) << std::endl << "[INFO] OpenGL renderer: " << glGetString(GL_RENDERER) << std::endl << "[INFO] OpenGL version: " << glGetString(GL_VERSION) << std::endl;

#ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(gl_debug_callback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    uint32_t vertex_shader_handle = compile_shader("particle.vert.spv", GL_VERTEX_SHADER);
    uint32_t fragment_shader_handle = compile_shader("particle.frag.spv", GL_FRAGMENT_SHADER);
    render_program_handle = glCreateProgram();
    glAttachShader(render_program_handle, fragment_shader_handle);
    glAttachShader(render_program_handle, vertex_shader_handle);
    glLinkProgram(render_program_handle);
    check_program_linked(render_program_handle);
    // delete shaders as we're done with them.
    glDeleteShader(vertex_shader_handle);
    glDeleteShader(fragment_shader_handle);

    uint32_t compute_shader_handle;
    compute_shader_handle = compile_shader("compute_density_pressure.comp.spv", GL_COMPUTE_SHADER);
    compute_program_handle[0] = glCreateProgram();
    glAttachShader(compute_program_handle[0], compute_shader_handle);
    glLinkProgram(compute_program_handle[0]);
    check_program_linked(render_program_handle);
    glDeleteShader(compute_shader_handle);

    compute_shader_handle = compile_shader("compute_force.comp.spv", GL_COMPUTE_SHADER);
    compute_program_handle[1] = glCreateProgram();
    glAttachShader(compute_program_handle[1], compute_shader_handle);
    glLinkProgram(compute_program_handle[1]);
    check_program_linked(render_program_handle);
    glDeleteShader(compute_shader_handle);

    compute_shader_handle = compile_shader("integrate.comp.spv", GL_COMPUTE_SHADER);
    compute_program_handle[2] = glCreateProgram();
    glAttachShader(compute_program_handle[2], compute_shader_handle);
    glLinkProgram(compute_program_handle[2]);
    check_program_linked(render_program_handle);
    glDeleteShader(compute_shader_handle);

    // ssbo sizes
    constexpr ptrdiff_t position_ssbo_size = sizeof(glm::vec2) * SPH_NUM_PARTICLES;
    constexpr ptrdiff_t velocity_ssbo_size = sizeof(glm::vec2) * SPH_NUM_PARTICLES;
    constexpr ptrdiff_t force_ssbo_size = sizeof(glm::vec2) * SPH_NUM_PARTICLES;
    constexpr ptrdiff_t density_ssbo_size = sizeof(float) * SPH_NUM_PARTICLES;
    constexpr ptrdiff_t pressure_ssbo_size = sizeof(float) * SPH_NUM_PARTICLES;

    constexpr ptrdiff_t packed_buffer_size = position_ssbo_size + velocity_ssbo_size + force_ssbo_size + density_ssbo_size + pressure_ssbo_size;
    // ssbo offsets
    constexpr ptrdiff_t position_ssbo_offset = 0;
    constexpr ptrdiff_t velocity_ssbo_offset = position_ssbo_size;
    constexpr ptrdiff_t force_ssbo_offset = velocity_ssbo_offset + velocity_ssbo_size;
    constexpr ptrdiff_t density_ssbo_offset = force_ssbo_offset + force_ssbo_size;
    constexpr ptrdiff_t pressure_ssbo_offset = density_ssbo_offset + density_ssbo_size;

    std::vector<glm::vec2> initial_position(SPH_NUM_PARTICLES);

    // test case 1
    if (scene_id == 0)
    {
        for (auto i = 0, x = 0, y = 0; i < SPH_NUM_PARTICLES; i++)
        {
            initial_position[i].x = -0.625f + SPH_PARTICLE_RADIUS * 2 * x;
            initial_position[i].y = 1 - SPH_PARTICLE_RADIUS * 2 * y;
            x++;
            if (x >= 125)
            {
                x = 0;
                y++;
            }
        }
    }
    // test case 2
    else
    {
        for (auto i = 0, x = 0, y = 0; i < SPH_NUM_PARTICLES; i++)
        {
            initial_position[i].x = -1 + SPH_PARTICLE_RADIUS * 2 * x;
            initial_position[i].y = -1 + SPH_PARTICLE_RADIUS * 2 * y;
            x++;
            if (x >= 100)
            {
                x = 0;
                y++;
            }
        }
    }
    void* initial_data = std::malloc(packed_buffer_size);
    std::memset(initial_data, 0, packed_buffer_size);
    std::memcpy(initial_data, initial_position.data(), position_ssbo_size);
    glGenBuffers(1, &packed_particles_buffer_handle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, packed_particles_buffer_handle);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, packed_buffer_size, initial_data, GL_DYNAMIC_STORAGE_BIT);
    std::free(initial_data);

    glGenVertexArrays(1, &particle_position_vao_handle);
    glBindVertexArray(particle_position_vao_handle);

    glBindBuffer(GL_ARRAY_BUFFER, packed_particles_buffer_handle);
    // bind buffer containing particle position to vao, stride is 0
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    // enable attribute with binding = 0 (vertex position in the shader) for this vao
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // bindings
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, packed_particles_buffer_handle, position_ssbo_offset, position_ssbo_size);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, packed_particles_buffer_handle, velocity_ssbo_offset, velocity_ssbo_size);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, packed_particles_buffer_handle, force_ssbo_offset, force_ssbo_size);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 3, packed_particles_buffer_handle, density_ssbo_offset, density_ssbo_size);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 4, packed_particles_buffer_handle, pressure_ssbo_offset, pressure_ssbo_size);

    glBindVertexArray(particle_position_vao_handle);

    // set clear color
    glClearColor(0.92f, 0.92f, 0.92f, 1.f);

}


void application::check_program_linked(GLuint shader_program_handle)
{
    int32_t is_linked = 0;
    glGetProgramiv(shader_program_handle, GL_LINK_STATUS, &is_linked);
    if (is_linked == GL_FALSE)
    {
        int32_t len = 0;
        glGetProgramiv(shader_program_handle, GL_INFO_LOG_LENGTH, &len);
        std::vector<GLchar> log(len);
        glGetProgramInfoLog(shader_program_handle, len, &len, &log[0]);

        for (const auto& el : log)
        {
            std::cout << el;
        }
        throw std::runtime_error("shader link error");
    }

}

GLuint application::compile_shader(std::string path_to_file, GLenum shader_type)
{
    GLuint shader_handle = 0;

    std::ifstream shader_file(path_to_file, std::ios::ate | std::ios::binary);
    if (!shader_file)
    {
        throw std::runtime_error("shader file load error");
    }
    size_t shader_file_size = (size_t)shader_file.tellg();
    std::vector<char> shader_code(shader_file_size);
    shader_file.seekg(0);
    shader_file.read(shader_code.data(), shader_file_size);
    shader_file.close();

    shader_handle = glCreateShader(shader_type);

    glShaderBinary(1, &shader_handle, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, shader_code.data(), static_cast<GLsizei>(shader_code.size()));
    glSpecializeShader(shader_handle, "main", 0, nullptr, nullptr);
    int32_t is_compiled = 0;
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &is_compiled);
    if (is_compiled == GL_FALSE)
    {
        int32_t len = 0;
        glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &len);
        std::vector<GLchar> log(len);
        glGetShaderInfoLog(shader_handle, len, &len, &log[0]);
        for (const auto& el : log)
        {
            std::cout << el;
        }
        throw std::runtime_error("shader compile error");
    }
    return shader_handle;
}

void application::main_loop()
{
    static std::chrono::high_resolution_clock::time_point frame_start;
    static std::chrono::high_resolution_clock::time_point frame_end;
    static int64_t total_frame_time_ns;

    frame_start = std::chrono::high_resolution_clock::now();

    // process user inputs
    glfwPollEvents();

    // step through the simulation if not paused
    if (!paused)
    {
        run_simulation();
        frame_number++;
    }

    render();

    glfwSwapBuffers(window);

    frame_end = std::chrono::high_resolution_clock::now();

    // measure performance
    total_frame_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(frame_end - frame_start).count();

    std::stringstream title;
    title.precision(3);
    title.setf(std::ios_base::fixed, std::ios_base::floatfield);
    title << "SPH Simulation (OpenGL) | "
        "particle count: " << SPH_NUM_PARTICLES << " | "
        "frame " << frame_number << " | "
        "frame time: " << 1e-6 * total_frame_time_ns << " ms | ";
    glfwSetWindowTitle(window, title.str().c_str());
}

void application::run_simulation()
{
    glUseProgram(compute_program_handle[0]);
    glDispatchCompute(SPH_NUM_WORK_GROUPS, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glUseProgram(compute_program_handle[1]);
    glDispatchCompute(SPH_NUM_WORK_GROUPS, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glUseProgram(compute_program_handle[2]);
    glDispatchCompute(SPH_NUM_WORK_GROUPS, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void application::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(render_program_handle);
    glDrawArrays(GL_POINTS, 0, SPH_NUM_PARTICLES);
}

} // namespace sph
