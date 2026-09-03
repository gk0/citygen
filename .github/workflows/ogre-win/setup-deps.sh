#!/bin/bash
# Fetch MSYS2 mingw64 packages (headers/import-libs/DLLs) and extract them
# into a prefix, so a Linux-hosted MinGW cross toolchain can link against
# the same binaries the native MSYS2 CI job will use.
# Usage: setup-deps.sh <prefix> <pkg> [<pkg>...]
set -euo pipefail

PREFIX="$1"; shift
REPO=https://repo.msys2.org/mingw/mingw64
TMP=$(mktemp -d)
mkdir -p "$PREFIX"

for pkg in "$@"; do
    f=$(curl -fsSL "$REPO/" | grep -oE "$pkg-[0-9][^\"']*\.pkg\.tar\.zst" | sort -u | sort -V | tail -1)
    [ -n "$f" ] || { echo "ERROR: no package found for $pkg" >&2; exit 1; }
    echo "fetching $f"
    curl -fsSLo "$TMP/$f" "$REPO/$f"
    tar --zstd -xf "$TMP/$f" -C "$PREFIX"
done

echo "MSYS2 deps installed to $PREFIX"
