#!/bin/sh
# Download this beta release and verify its published checksum.
set -eu
command -v curl >/dev/null || { echo 'Install curl first.' >&2; exit 1; }
command -v sha256sum >/dev/null || { echo 'Install coreutils first.' >&2; exit 1; }
release_version=0.25.0-beta.1
release_url=https://github.com/scolapvsta/WallpaperLab/releases/download/v$release_version
installer_name=Wallpaper-Lab-$release_version-Arch-x86_64.run
work_dir=$(mktemp -d)
trap 'rm -rf "$work_dir"' EXIT HUP INT TERM
cd "$work_dir"
curl --fail --location --proto '=https' --tlsv1.2 -o installer.run "$release_url/$installer_name"
curl --fail --location --proto '=https' --tlsv1.2 -o SHA256SUMS "$release_url/SHA256SUMS"
awk -v name="$installer_name" '$2 == name {print $1 "  installer.run"}' SHA256SUMS > installer.sha256
[ -s installer.sha256 ] || { echo 'Installer checksum missing.' >&2; exit 1; }
sha256sum -c installer.sha256
bash installer.run "$@"
