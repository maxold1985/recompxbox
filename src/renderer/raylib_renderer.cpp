#include "recompxbox/raylib_renderer.h"
#include "raylib.h"
#include "rlgl.h"

namespace recompxbox {
void renderFrameRaylib(const Frame& frame) {
    BeginDrawing();
    ClearBackground(::Color{frame.clear.r, frame.clear.g, frame.clear.b, frame.clear.a});
    // Demonstration backend: vertices are interpreted as pixel coordinates.
    // A real Xbox renderer needs viewport, projection, depth and shader semantics.
    rlMatrixMode(RL_PROJECTION);
    rlPushMatrix();
    rlLoadIdentity();
    rlOrtho(0, GetScreenWidth(), GetScreenHeight(), 0, -1, 1);
    rlMatrixMode(RL_MODELVIEW);
    rlPushMatrix();
    rlLoadIdentity();
    rlViewport(frame.viewport.x, frame.viewport.y, frame.viewport.width, frame.viewport.height);
    for (const auto& draw : frame.draws) {
        rlBegin(draw.primitive == Primitive::Triangles ? RL_TRIANGLES : RL_LINES);
        for (const auto& v : draw.vertices) {
            rlColor4ub(v.color.r, v.color.g, v.color.b, v.color.a);
            rlVertex3f(v.x, v.y, v.z);
        }
        rlEnd();
    }
    rlDrawRenderBatchActive();
    rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());
    rlPopMatrix();
    rlMatrixMode(RL_PROJECTION);
    rlPopMatrix();
    rlMatrixMode(RL_MODELVIEW);
    EndDrawing();
}
} // namespace recompxbox
