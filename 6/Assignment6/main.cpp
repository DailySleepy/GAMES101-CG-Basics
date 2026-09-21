#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>
#include <fstream>
#include <string>

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char** argv)
{
    Scene scene(1280, 960);

    std::string bunny_path = "models/bunny/bunny.obj";
    {
        std::ifstream check_file(bunny_path);
        if (!check_file.good()) {
            bunny_path = "../models/bunny/bunny.obj";
        }
    }
    MeshTriangle bunny(bunny_path);


    scene.Add(&bunny);
    scene.Add(std::make_unique<Light>(Vector3f(-20, 70, 20), 1));
    scene.Add(std::make_unique<Light>(Vector3f(20, 70, 20), 1));
    scene.buildBVH();

    Renderer r;

    auto start = std::chrono::system_clock::now();
    r.Render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "Render complete: \n";
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    std::cout << "Time taken: " << duration_ms << " ms (" << duration_ms / 1000.0 << " seconds)\n";

    return 0;
}