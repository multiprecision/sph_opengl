/*
* Copyright (c) 2017, Samuel I. Gunadi
* All rights reserved.
*/

#include "application.hpp"

#include <cmath>
#include <string>
#include <algorithm>
#include <exception>

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include <thread>

#ifdef _DEBUG
void APIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* data)
{
    char* source_str;
    char* type_str;
    char* severity_str;

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
    glDeleteBuffers(1, &particle_buffer_handle);

}

void application::run()
{
    // to measure performance
    std::thread(
        [this]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20000));
            std::cout << "[INFO] frame count after 20 seconds after setup (do not pause or move the window): " << frame_number << std::endl;
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
    glfwSetWindowPos(window, 0, 0);
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
    glewInit();
    if(!glewIsSupported("GL_VERSION_4_5"))
    {
        throw std::runtime_error("need opengl 4.5 core");
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

    uint32_t vertex_shader_handle = compile_shader("../shader/particle.vert", GL_VERTEX_SHADER);
    uint32_t fragment_shader_handle = compile_shader("../shader/particle.frag", GL_FRAGMENT_SHADER);
    render_program_handle = glCreateProgram();
    glAttachShader(render_program_handle, fragment_shader_handle);
    glAttachShader(render_program_handle, vertex_shader_handle);
    glLinkProgram(render_program_handle);
    check_program_linked(render_program_handle);
    // delete shaders as we're done with them.
    glDeleteShader(vertex_shader_handle);
    glDeleteShader(fragment_shader_handle);

    uint32_t compute_shader_handle;
    compute_shader_handle = compile_shader("../shader/compute_density_pressure.comp", GL_COMPUTE_SHADER);
    compute_program_handle[0] = glCreateProgram();
    glAttachShader(compute_program_handle[0], compute_shader_handle);
    glLinkProgram(compute_program_handle[0]);
    check_program_linked(render_program_handle);
    glDeleteShader(compute_shader_handle);

    compute_shader_handle = compile_shader("../shader/compute_force.comp", GL_COMPUTE_SHADER);
    compute_program_handle[1] = glCreateProgram();
    glAttachShader(compute_program_handle[1], compute_shader_handle);
    glLinkProgram(compute_program_handle[1]);
    check_program_linked(render_program_handle);
    glDeleteShader(compute_shader_handle);

    compute_shader_handle = compile_shader("../shader/integrate.comp", GL_COMPUTE_SHADER);
    compute_program_handle[2] = glCreateProgram();
    glAttachShader(compute_program_handle[2], compute_shader_handle);
    glLinkProgram(compute_program_handle[2]);
    check_program_linked(render_program_handle);
    glDeleteShader(compute_shader_handle);

    particle_type initial_particle_data[SPH_PARTICLE_COUNT];
    // initialize to zero
    std::memset(initial_particle_data, 0, sizeof(particle_type) * SPH_PARTICLE_COUNT);

    // test case 1
    if (scene_id == 0)
    {
        for (auto i = 0, x = 0, y = 0; i < SPH_PARTICLE_COUNT; i++)
        {
            initial_particle_data[i].position.x = -0.625f + SPH_PARTICLE_RADIUS * 2 * x;
            initial_particle_data[i].position.y = 1 - SPH_PARTICLE_RADIUS * 2 * y;
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
        for (auto i = 0, x = 0, y = 0; i < SPH_PARTICLE_COUNT; i++)
        {
            initial_particle_data[i].position.x = -1 + SPH_PARTICLE_RADIUS * 2 * x;
            initial_particle_data[i].position.y = -1 + SPH_PARTICLE_RADIUS * 2 * y;
            x++;
            if (x >= 100)
            {
                x = 0;
                y++;
            }
        }
    }
    glGenBuffers(1, &particle_buffer_handle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particle_buffer_handle);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(particle_type) * SPH_PARTICLE_COUNT, initial_particle_data, GL_MAP_READ_BIT);


    glGenVertexArrays(1, &particle_position_vao_handle);
    glBindVertexArray(particle_position_vao_handle);

    glBindBuffer(GL_ARRAY_BUFFER, particle_buffer_handle);
    // bind buffer containing particle position to vao, stride is sizeof(particle_t)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(particle_type), nullptr);
    // enable attribute with binding = 0 (vertex position in the shader) for this vao
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0); // release

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

        for (auto c : log)
        {
            std::cout << c;
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
    size_t shader_file_size = static_cast<size_t>(shader_file.tellg());
    std::vector<char> shader_code(shader_file_size);
    shader_file.seekg(0);
    shader_file.read(shader_code.data(), shader_file_size);
    shader_file.close();
    // must be NUL terminated
    shader_code.push_back('\0');

    char* shader_code_ptr = shader_code.data();
    shader_handle = glCreateShader(shader_type);
    glShaderSource(shader_handle, 1, &shader_code_ptr, NULL);
    glCompileShader(shader_handle);

    int32_t is_compiled = 0;
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &is_compiled);
    if (is_compiled == GL_FALSE)
    {
        int32_t len = 0;
        glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &len);
        std::vector<GLchar> log(len);
        glGetShaderInfoLog(shader_handle, len, &len, &log[0]);
        for (auto c : log)
        {
            std::cout << c;
        }
        throw std::runtime_error("shader compile error");
    }
    return shader_handle;
}

void application::main_loop()
{

    frame_start = std::chrono::steady_clock::now();

    // process user inputs
    glfwPollEvents();

    // step through the simulation if not paused
    if (!paused)
    {
        step_forward();
    }

    render();

    // measure performance
    frame_end = std::chrono::steady_clock::now();
    frame_time = std::chrono::duration_cast<std::chrono::duration<double>>(frame_end - frame_start).count();
    std::stringstream ss;
    ss.precision(3);
    ss.setf(std::ios_base::fixed, std::ios_base::floatfield);
    ss << "SPH Simulation (OpenGL) | frame " << frame_number << " | t: " << SPH_TIME_STEP * frame_number << " | frame time: " << frame_time * 1000 << " ms | particle count: " << SPH_PARTICLE_COUNT;
    glfwSetWindowTitle(window, ss.str().c_str());

    if (!paused)
    {
        frame_number++;
    }
}

void application::step_forward()
{

    for (size_t i = 0; i < 3; i++)
    {
        glUseProgram(compute_program_handle[i]);
        glBindBuffersBase(GL_SHADER_STORAGE_BUFFER, 0, 1, &particle_buffer_handle);
        glDispatchCompute(SPH_GROUP_COUNT, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

void application::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(render_program_handle);
    glBindVertexArray(particle_position_vao_handle);
    glDrawArrays(GL_POINTS, 0, SPH_PARTICLE_COUNT); // draw particle as points
    glBindVertexArray(0);

    glfwSwapBuffers(window);

}

} // namespace sph