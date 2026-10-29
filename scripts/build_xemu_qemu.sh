#!/usr/bin/env bash
# Build the Xbox-capable xemu QEMU fork separately from recompxbox.
# No firmware or copyrighted game assets are downloaded.
set -euo pipefail
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source_dir="${XEMU_SOURCE_DIR:-$repo_root/third_party/xemu}"
jobs="${XEMU_JOBS:-2}"
if [[ ! -f "$source_dir/configure" || ! -f "$source_dir/build.sh" ]]; then
  echo "xemu sources missing at $source_dir" >&2
  echo "Clone: git clone --recursive https://github.com/xemu-project/xemu.git $source_dir" >&2
  exit 2
fi
source_dir="$(cd "$source_dir" && pwd)"
# xemu's own build.sh sets -DXBOX=1 and builds the i386-softmmu target.
cd "$source_dir"
./build.sh "-j$jobs"
if [[ ! -x "$source_dir/build/qemu-system-i386" ]]; then
  echo "xemu build did not produce build/qemu-system-i386" >&2
  exit 1
fi
# xemu initializes SDL even for --version. Run it under a virtual display
# on headless CI hosts; on a desktop keep the normal SDL environment.
if [[ -z "${DISPLAY:-}" && -z "${WAYLAND_DISPLAY:-}" ]] && command -v xvfb-run >/dev/null 2>&1; then
  SDL_VIDEODRIVER=x11 xvfb-run -a "$source_dir/build/qemu-system-i386" --version
else
  "$source_dir/build/qemu-system-i386" --version
fi
echo "Xbox-capable xemu QEMU built: $source_dir/build/qemu-system-i386"
echo "Note: this is a separate emulator, not linked to recompxbox raylib."
