# D3D8 HLE → raylib (experimental)

This is a **host-side prototype**, not an implementation of the Xbox D3D8 ABI.
It records clear, viewport and non-indexed triangle/line draw commands, then
replays them using raylib/rlgl. It does not intercept XBE calls.

## Build and test without raylib

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Optional raylib demo

Install raylib with CMake package config, then:

```sh
cmake -S . -B build -DRECOMPXBOX_BUILD_RAYLIB=ON
cmake --build build
./build/recompxbox_raylib_demo
```

## Not yet implemented

- Xbox D3D8 function interception and calling conventions
- XBE loader, kernel and guest memory mapping
- Textures, indexed draws, render states, depth/stencil and blending
- Fixed-function pipeline, NV2A push buffers and Xbox shaders
- D3D9 interface, compatibility with real Xbox games

These require substantial additional work. Cxbx-Reloaded, xemu and XQEMU
remain source references as Git submodules, not linked dependencies.
