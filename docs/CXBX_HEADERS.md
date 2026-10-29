# Original Cxbx-Reloaded header integration

The Windows-only target `recompxbox_cxbx_header_check` includes
`third_party/Cxbx-Reloaded/src/core/hle/D3D8/XbD3D8Types.h` directly
and compiles `src/hle/d3d8/cxbx_header_compile.cpp`. Compile-time
assertions compare all supported primitive enum IDs with upstream.
The typed overload takes `XTL::X_D3DPRIMITIVETYPE`.

This upstream header imports `d3d9.h`, `d3dx9math.h`,
`d3dx9tex.h` and `dxerr9.h`, and therefore needs the legacy
DirectX SDK (June 2010) and a Windows development environment.
CMake checks that these headers exist when the option is enabled.

Build (Windows, DirectX SDK installed and INCLUDE configured):
```powershell
git submodule update --init third_party/Cxbx-Reloaded
cmake -S . -B build -DRECOMPXBOX_USE_CXBX_HEADERS=ON
cmake --build build --config Release
```

Portable Linux CI intentionally builds the independent adapter without
including Windows-specific upstream headers. The original Cxbx headers
are *not* rewritten or copied into this repository.

This integration verifies source-level type compatibility; it does not
link or invoke the Cxbx emulator, intercept an XBE, or execute games.
The Cxbx project is GPL-2.0-or-later. Preserve license requirements
if integrating its source code into a distributed combined work.
