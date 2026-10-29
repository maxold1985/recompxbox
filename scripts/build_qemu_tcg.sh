#!/usr/bin/env bash
# Build upstream QEMU with x86 TCG enabled, separately from the HLE runtime.
set -euo pipefail
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
src="$root/third_party/qemu"
build="${1:-$root/build-qemu-tcg}"
if [[ ! -f "$src/configure" ]]; then
  echo "QEMU submodule missing: git submodule update --init third_party/qemu" >&2
  exit 2
fi
mkdir -p "$build"
build="$(cd "$build" && pwd)"
cd "$build"
"$src/configure" --target-list=i386-softmmu --enable-tcg --disable-kvm --disable-xen --disable-gtk --disable-sdl --disable-curses --disable-vnc --disable-docs --disable-werror
cmake --version >/dev/null
ninja qemu-system-i386
"$build/qemu-system-i386" --version
echo "QEMU TCG backend built. XBE execution is not wired to this binary."
