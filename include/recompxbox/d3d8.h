#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace recompxbox {
// Host-independent subset of D3D8 state. Not Xbox binary ABI-compatible.
struct Color { std::uint8_t r, g, b, a; };
struct Vertex { float x, y, z; Color color; };
struct Viewport { int x = 0, y = 0, width = 640, height = 480; };
enum class Primitive { Triangles, Lines };
struct DrawCommand {
    Primitive primitive = Primitive::Triangles;
    std::vector<Vertex> vertices;
};
struct Frame {
    Color clear = {0, 0, 0, 255};
    Viewport viewport;
    std::vector<DrawCommand> draws;
};
class D3D8Device {
public:
    bool beginScene();
    bool endScene();
    void clear(Color color);
    bool setViewport(Viewport viewport);
    bool drawPrimitive(Primitive primitive, const Vertex* vertices, std::size_t count);
    // Copies the completed frame; rendering happens on the host renderer.
    bool present(Frame& output);
private:
    bool sceneActive_ = false;
    Frame pending_;
};
} // namespace recompxbox
