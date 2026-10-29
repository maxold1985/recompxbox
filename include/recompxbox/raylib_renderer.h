#pragma once
#include "recompxbox/d3d8.h"

namespace recompxbox {
// Call from the thread owning the raylib window, between InitWindow and CloseWindow.
void renderFrameRaylib(const Frame& frame);
}
