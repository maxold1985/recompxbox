#include "raylib.h"
#include "recompxbox/d3d8.h"
#include "recompxbox/raylib_renderer.h"

int main() {
    InitWindow(800, 600, "RecompXbox D3D8 HLE prototype");
    SetTargetFPS(60);
    recompxbox::D3D8Device device;
    device.setViewport({0, 0, 800, 600});
    const recompxbox::Vertex triangle[] = {
        {400, 100, 0, {255, 0, 0, 255}},
        {150, 500, 0, {0, 255, 0, 255}},
        {650, 500, 0, {0, 0, 255, 255}}
    };
    while (!WindowShouldClose()) {
        device.clear({20, 25, 40, 255});
        device.beginScene();
        device.drawPrimitive(recompxbox::Primitive::Triangles, triangle, 3);
        device.endScene();
        recompxbox::Frame frame;
        if (device.present(frame)) recompxbox::renderFrameRaylib(frame);
    }
    CloseWindow();
}
