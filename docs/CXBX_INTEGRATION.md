# Cxbx-Reloaded integration, phase 1

Source reference: `third_party/Cxbx-Reloaded/src/core/hle/D3D8/XbD3D8Types.h`.
The Xbox primitive enum `X_D3DPRIMITIVETYPE` defines numeric IDs 1..10,
including Xbox-specific line loops and quad topologies.

The `translateCxbxPrimitive` adapter **consumes the same numeric command
representation** as Cxbx's D3D8 HLE and converts supported topologies to
the existing raylib triangle/line command queue. This is the first
source-informed integration, not merely a submodule link.

The original Cxbx headers require DirectX 9 SDK and Windows-specific types.
For that reason this portable adapter intentionally does not include them.
A later Windows-specific bridge can include the upstream headers and
`static_assert` the enum values to detect drift.

**Not implemented:** calling Cxbx's runtime, intercepting XBE graphics calls,
NV2A state, GPU memory, texture decoding, shaders, and actual game execution.

Cxbx-Reloaded is GPL-2.0-or-later; keep its original licensing and attribution
when incorporating source files. The current adapter is separately written
using the public enum values and does not copy Cxbx source files.
