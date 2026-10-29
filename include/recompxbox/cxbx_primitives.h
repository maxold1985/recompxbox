#pragma once
#include "recompxbox/d3d8.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace recompxbox {
// Numeric primitive IDs verified against Cxbx-Reloaded's
// src/core/hle/D3D8/XbD3D8Types.h (X_D3DPRIMITIVETYPE).
// This is an adapter for Cxbx's Xbox-specific API representation,
// NOT a binary link against the emulator or an XBE hook.
enum class XboxPrimitive : std::uint32_t {
    PointList = 1, LineList = 2, LineLoop = 3, LineStrip = 4,
    TriangleList = 5, TriangleStrip = 6, TriangleFan = 7,
    QuadList = 8, QuadStrip = 9, Polygon = 10
};
// Converts Xbox primitive topology to the existing raylib command queue.
// Rejects unsupported point/polygon topology instead of silently drawing wrong geometry.
bool translateCxbxPrimitive(std::uint32_t xboxType, const Vertex* input,
                            std::size_t count, DrawCommand& output);
}
