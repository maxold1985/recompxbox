#pragma once
// Compile-time bridge to the ORIGINAL Cxbx-Reloaded D3D8 definitions.
// Enabled only for Windows builds with the legacy DirectX SDK installed.
// Portable builds continue using the stable numeric adapter.
#if !defined(RECOMPXBOX_USE_CXBX_HEADERS)
#error "Enable RECOMPXBOX_USE_CXBX_HEADERS via CMake before including this bridge."
#endif
#if !defined(_WIN32)
#error "Cxbx D3D8 headers require Windows/DirectX SDK."
#endif
#include "core/hle/D3D8/XbD3D8Types.h"
#include "recompxbox/cxbx_primitives.h"

namespace recompxbox {
// The Xbox values must match the upstream Cxbx header exactly.
static_assert(static_cast<std::uint32_t>(XTL::X_D3DPT_POINTLIST) == 1, "Cxbx enum drift");
static_assert(static_cast<std::uint32_t>(XTL::X_D3DPT_LINELIST) == 2, "Cxbx enum drift");
static_assert(static_cast<std::uint32_t>(XTL::X_D3DPT_LINELOOP) == 3, "Cxbx enum drift");
static_assert(static_cast<std::uint32_t>(XTL::X_D3DPT_LINESTRIP) == 4, "Cxbx enum drift");
static_assert(static_cast<std::uint32_t>(XTL::X_D3DPT_TRIANGLELIST) == 5, "Cxbx enum drift");
static_assert(static_cast<std::uint32_t>(XTL::X_D3DPT_TRIANGLESTRIP) == 6, "Cxbx enum drift");
static_assert(static_cast<std::uint32_t>(XTL::X_D3DPT_TRIANGLEFAN) == 7, "Cxbx enum drift");
static_assert(static_cast<std::uint32_t>(XTL::X_D3DPT_QUADLIST) == 8, "Cxbx enum drift");
static_assert(static_cast<std::uint32_t>(XTL::X_D3DPT_QUADSTRIP) == 9, "Cxbx enum drift");
static_assert(static_cast<std::uint32_t>(XTL::X_D3DPT_POLYGON) == 10, "Cxbx enum drift");

// Typed entrypoint: callers pass the actual Cxbx enum, not a copied enum.
inline bool translateCxbxPrimitive(XTL::X_D3DPRIMITIVETYPE type,
                                    const Vertex* vertices, std::size_t count,
                                    DrawCommand& output) {
    return translateCxbxPrimitive(static_cast<std::uint32_t>(type), vertices, count, output);
}
}
