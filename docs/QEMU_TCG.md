# QEMU TCG integration — phase 1

The upstream QEMU source is pinned as a Git submodule at
`third_party/qemu`. The opt-in `recompxbox_qemu_tcg` target builds
**real upstream QEMU x86 system emulation with TCG enabled** in a separate
build directory.

This is an integration of the **build dependency and testable CPU engine
artifact**, not yet an in-process TCG backend for XBE code. QEMU's TCG
is tightly coupled to QEMU CPU state, translation blocks, memory
subsystems, exceptions, and device models. Merely linking its `tcg/`
source files to a CMake library would not make it functional.

## Linux or WSL2

Install the build dependencies for upstream QEMU (including Meson, Ninja,
Python, pkg-config, GLib and pixman development packages), then:

```bash
git submodule update --init third_party/qemu
cmake -S . -B build -DRECOMPXBOX_BUILD_QEMU_TCG=ON
cmake --build build --target recompxbox_qemu_tcg
./build/qemu-tcg/qemu-system-i386 --version
```

The normal `recompxbox` CLI and its tests remain independent of QEMU.
The optional build is not run in default CI because upstream QEMU has
additional dependencies and significant compilation cost.

## Next integration milestone

Define a guest CPU API (registers, guest address space, memory callbacks,
exceptions, instruction budget, exit/trap callbacks). Integrate the
QEMU i386 CPU translation/execution loop behind that API, map XBE sections
into the guest address space, and dispatch Xbox kernel calls. Validate with
synthetic x86 programs before attempting a game.

**Current status:** no XBE code is executed by QEMU, and no Xbox game
compatibility is claimed. The existing `x86_probe` is only a test utility.

QEMU source is GPL-2.0-or-later with component-specific licensing.
Review licenses before redistributing combined binaries.
