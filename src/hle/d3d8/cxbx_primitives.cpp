#include "recompxbox/cxbx_primitives.h"
#include <utility>

namespace recompxbox {
bool translateCxbxPrimitive(std::uint32_t type, const Vertex* in,
                            std::size_t n, DrawCommand& out) {
    if (!in || n == 0) return false;
    DrawCommand tmp;
    auto emit = [&](std::size_t a, std::size_t b, std::size_t c) {
        tmp.vertices.push_back(in[a]);
        tmp.vertices.push_back(in[b]);
        tmp.vertices.push_back(in[c]);
    };
    switch (static_cast<XboxPrimitive>(type)) {
    case XboxPrimitive::LineList:
        if (n % 2 != 0) return false;
        tmp.primitive = Primitive::Lines;
        tmp.vertices.assign(in, in + n);
        break;
    case XboxPrimitive::LineStrip:
    case XboxPrimitive::LineLoop:
        if (n < 2) return false;
        tmp.primitive = Primitive::Lines;
        for (std::size_t i = 1; i < n; ++i) {
            tmp.vertices.push_back(in[i - 1]);
            tmp.vertices.push_back(in[i]);
        }
        if (type == static_cast<std::uint32_t>(XboxPrimitive::LineLoop)) {
            tmp.vertices.push_back(in[n - 1]);
            tmp.vertices.push_back(in[0]);
        }
        break;
    case XboxPrimitive::TriangleList:
        if (n % 3 != 0) return false;
        tmp.vertices.assign(in, in + n);
        break;
    case XboxPrimitive::TriangleStrip:
        if (n < 3) return false;
        for (std::size_t i = 2; i < n; ++i) {
            if (i % 2 == 0) emit(i - 2, i - 1, i);
            else emit(i - 1, i - 2, i);
        }
        break;
    case XboxPrimitive::TriangleFan:
        if (n < 3) return false;
        for (std::size_t i = 2; i < n; ++i) emit(0, i - 1, i);
        break;
    case XboxPrimitive::QuadList:
        if (n % 4 != 0) return false;
        for (std::size_t i = 0; i < n; i += 4) {
            emit(i, i + 1, i + 2);
            emit(i, i + 2, i + 3);
        }
        break;
    case XboxPrimitive::QuadStrip:
        if (n < 4 || n % 2 != 0) return false;
        for (std::size_t i = 0; i + 3 < n; i += 2) {
            emit(i, i + 1, i + 3);
            emit(i, i + 3, i + 2);
        }
        break;
    default:
        return false;
    }
    out = std::move(tmp);
    return true;
}
}
