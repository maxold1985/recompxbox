#include "recompxbox/cxbx_primitives.h"
#include <cassert>

int main() {
    using namespace recompxbox;
    const Vertex v[6] = {
        {0,0,0,{}}, {1,0,0,{}}, {2,0,0,{}},
        {3,0,0,{}}, {4,0,0,{}}, {5,0,0,{}}
    };
    DrawCommand out;
    assert(translateCxbxPrimitive(5, v, 6, out) && out.vertices.size() == 6);
    assert(translateCxbxPrimitive(6, v, 4, out) && out.vertices.size() == 6);
    assert(out.vertices[3].x == 2 && out.vertices[4].x == 1);
    assert(translateCxbxPrimitive(7, v, 4, out) && out.vertices.size() == 6);
    assert(translateCxbxPrimitive(8, v, 4, out) && out.vertices.size() == 6);
    assert(translateCxbxPrimitive(9, v, 6, out) && out.vertices.size() == 12);
    assert(translateCxbxPrimitive(3, v, 4, out) && out.vertices.size() == 8);
    assert(!translateCxbxPrimitive(1, v, 4, out));
    assert(!translateCxbxPrimitive(99, v, 4, out));
}
