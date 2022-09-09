# SPH Fluid Simulation in OpenGL
[![Build status](https://ci.appveyor.com/api/projects/status/bdmh929h1m7ir32l?svg=true)](https://ci.appveyor.com/project/multiprecision/sph-opengl)

Smoothed Particle Hydrodynamics implementation in OpenGL compute shader. Licensed under MIT License.

## Further reading
https://github.com/multiprecision/undergraduate_thesis/blob/master/undergraduate_thesis.pdf

## Quickstart guide
1. Install [Visual Studio 2022](https://visualstudio.microsoft.com/) with "Desktop development with C++" workload and "Windows 11 SDK (10.0.22000)" component.
2. Make sure to have the latest graphics driver installed.
3. Install the latest [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) (version 1.3) and select GLM during installation.
4. Install [Python 3](https://www.python.org/downloads/) to run shader compilation script.
5. Run compile.py to compile shaders.
6. Open sph.sln, build, and run.

## Third-party libraries
1. [Vulkan SDK (GLM is bundled)](https://vulkan.lunarg.com/sdk/home)
2. [GLFW (bundled in the third_party folder)](https://github.com/glfw/glfw)
3. [gl3w (bundled)](https://github.com/skaslev/gl3w)

## Short video
https://www.youtube.com/watch?v=4LnaZmim81k

## Vulkan version
https://github.com/multiprecision/sph_vulkan
