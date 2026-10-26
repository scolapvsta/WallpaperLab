#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."
git submodule update --init --recursive
apply_patch_if_needed() {
    local directory="$1" patch_file="$2"
    if git -C "$directory" apply --check "$PWD/$patch_file" 2>/dev/null; then
        git -C "$directory" apply "$PWD/$patch_file"
    else
        git -C "$directory" apply --reverse --check "$PWD/$patch_file"
    fi
}
apply_patch_if_needed vendor/engine patches/engine-integration.patch
apply_patch_if_needed vendor/raylib patches/raylib-app-id.patch
prefix_args=()
if [[ -d local/sysroot/usr ]]; then prefix_args+=("-DCMAKE_PREFIX_PATH=$PWD/local/sysroot/usr"); fi
cmake -S vendor/engine -B build-engine -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF "${prefix_args[@]}"
cmake --build build-engine --target linux-wallpaperengine-lib -j "${BUILD_JOBS:-6}"
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release "${prefix_args[@]}"
cmake --build build -j "${BUILD_JOBS:-6}"
ctest --test-dir build --output-on-failure
