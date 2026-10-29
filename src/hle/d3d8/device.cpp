#include "recompxbox/d3d8.h"
#include <utility>

namespace recompxbox {
bool D3D8Device::beginScene() {
    if (sceneActive_) return false;
    sceneActive_ = true;
    return true;
}
bool D3D8Device::endScene() {
    if (!sceneActive_) return false;
    sceneActive_ = false;
    return true;
}
void D3D8Device::clear(Color color) { pending_.clear = color; }
bool D3D8Device::setViewport(Viewport viewport) {
    if (viewport.width <= 0 || viewport.height <= 0) return false;
    pending_.viewport = viewport;
    return true;
}
bool D3D8Device::drawPrimitive(Primitive primitive, const Vertex* vertices, std::size_t count) {
    if (!sceneActive_ || !vertices || count == 0) return false;
    const std::size_t group = primitive == Primitive::Triangles ? 3 : 2;
    if (count % group != 0) return false;
    pending_.draws.push_back({primitive, std::vector<Vertex>(vertices, vertices + count)});
    return true;
}
bool D3D8Device::present(Frame& output) {
    if (sceneActive_) return false;
    output = std::move(pending_);
    pending_ = Frame{};
    return true;
}
} // namespace recompxbox
