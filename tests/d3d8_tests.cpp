#include "recompxbox/d3d8.h"
#include <cassert>

int main() {
    recompxbox::D3D8Device device;
    recompxbox::Frame frame;
    const recompxbox::Vertex vertices[3] = {};
    assert(!device.endScene());
    assert(!device.setViewport({0, 0, 0, 480}));
    assert(device.setViewport({0, 0, 800, 600}));
    assert(device.beginScene());
    assert(!device.beginScene());
    assert(!device.drawPrimitive(recompxbox::Primitive::Triangles, vertices, 2));
    assert(device.drawPrimitive(recompxbox::Primitive::Triangles, vertices, 3));
    assert(!device.present(frame));
    assert(device.endScene());
    assert(device.present(frame));
    assert(frame.draws.size() == 1);
    assert(frame.viewport.width == 800);
    assert(device.present(frame));
    assert(frame.draws.empty());
}
