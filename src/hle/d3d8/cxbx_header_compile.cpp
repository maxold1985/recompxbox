// Compile this translation unit to verify the actual third-party headers.
// This is a compile-time compatibility check, not an emulator link.
#include "recompxbox/cxbx_header_bridge.h"
namespace recompxbox {
bool cxbxHeaderBridgeSmokeTest() {
    const Vertex v[3] = {};
    DrawCommand cmd;
    return translateCxbxPrimitive(XTL::X_D3DPT_TRIANGLELIST, v, 3, cmd);
}
}
