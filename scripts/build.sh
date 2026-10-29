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
python3 scripts/apply-patch-series.py vendor/engine \
    patches/engine-integration.patch patches/engine-build-paths.patch \
    patches/engine-audio-web.patch patches/engine-display-layout.patch \
    patches/engine-wallpaper-placement.patch patches/engine-live-controls.patch \
    patches/engine-scene-interaction.patch patches/engine-project-controls.patch \
    patches/engine-web-compatibility.patch patches/engine-sound-controls.patch \
    patches/engine-transitions-media-text.patch \
    patches/engine-stable-transitions-image-controls.patch \
    patches/engine-custom-animation-image-transform.patch \
    patches/engine-transition-handoff.patch \
    patches/engine-compositor-transitions.patch
apply_patch_if_needed vendor/raylib patches/raylib-app-id.patch
prefix_args=()
if [[ -d local/sysroot/usr ]]; then prefix_args+=("-DCMAKE_PREFIX_PATH=$PWD/local/sysroot/usr"); fi
cmake -S vendor/engine -B build-engine -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF "${prefix_args[@]}"
cmake --build build-engine --target linux-wallpaperengine-lib -j "${BUILD_JOBS:-6}"
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release "${prefix_args[@]}"
cmake --build build -j "${BUILD_JOBS:-6}"
ctest --test-dir build --output-on-failure
