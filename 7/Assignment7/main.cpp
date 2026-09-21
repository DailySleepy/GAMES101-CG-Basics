#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"
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

    // Change the definition here to change resolution
    Scene scene(784, 784);

    Material* red = new Material(DIFFUSE, Vector3f(0.0f));
    red->Kd = Vector3f(0.63f, 0.065f, 0.05f);
    Material* green = new Material(DIFFUSE, Vector3f(0.0f));
    green->Kd = Vector3f(0.14f, 0.45f, 0.091f);
    Material* white = new Material(DIFFUSE, Vector3f(0.0f));
    white->Kd = Vector3f(0.725f, 0.71f, 0.68f);
    Material* light = new Material(DIFFUSE, (8.0f * Vector3f(0.747f+0.058f, 0.747f+0.258f, 0.747f) + 15.6f * Vector3f(0.740f+0.287f,0.740f+0.160f,0.740f) + 18.4f *Vector3f(0.737f+0.642f,0.737f+0.159f,0.737f)));
    light->Kd = Vector3f(0.65f);

    Material* gold = new Material(MICROFACET, Vector3f(0.0f));
    gold->Kd = Vector3f(1.0f, 0.71f, 0.29f); // 黄金固有色
    gold->roughness = 0.35f;                 // 粗糙度
    gold->metallic = 1.0f;                  // 纯金属导体

    std::string model_dir = "models/cornellbox/";
    {
        std::ifstream check_file(model_dir + "floor.obj");
        if (!check_file.good()) {
            model_dir = "../models/cornellbox/";
        }
    }

    std::string bunny_path = "models/bunny/bunny.obj";
    {
        std::ifstream check_file(bunny_path);
        if (!check_file.good()) {
            bunny_path = "../models/bunny/bunny.obj";
        }
    }

    MeshTriangle floor(model_dir + "floor.obj", white);
    MeshTriangle shortbox(model_dir + "shortbox.obj", white);
    MeshTriangle tallbox(model_dir + "tallbox.obj", white);
    MeshTriangle left(model_dir + "left.obj", red);
    MeshTriangle right(model_dir + "right.obj", green);
    MeshTriangle light_(model_dir + "light.obj", light);
    MeshTriangle bunny(bunny_path, gold, Vector3f(200.0f, -50.0f, 200.0f), Vector3f(1500.0f, 1500.0f, 1500.0f), 180.0f);
    Sphere sphere(Vector3f(350, 100, 350), 100, white);

    scene.Add(&floor);
    //scene.Add(&shortbox);
    //scene.Add(&tallbox);
    scene.Add(&left);
    scene.Add(&right);
    scene.Add(&light_);
    scene.Add(&bunny);
    //scene.Add(&sphere);

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