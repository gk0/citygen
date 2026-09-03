#!/bin/bash
# Trial build of OGRE 1.6.5 (Windows MinGW) inside the citygen-cross container.
# Mirrors what .github/workflows/windows.yml will do natively: pinned tarball -> patches ->
# CMake port. Usage: trial-build.sh [cmake --build args...]
set -euo pipefail

OGRE_URL="https://downloads.sourceforge.net/project/ogre/ogre/1.6.5/ogre-v1-6-5.tar.bz2"
OGRE_SHA256="7fc0e948679c1c1f10751756d267a41d0e3395a6520a23f7853a0ae39a1281f5"

# 1. OGRE source from the pinned tarball (cached across runs)
if [ ! -f /work/ogre-src/OgreMain/include/OgreRoot.h ]; then
    curl -fsSL -o /tmp/ogre.tar.bz2 "$OGRE_URL"
    echo "$OGRE_SHA256  /tmp/ogre.tar.bz2" | sha256sum -c -
    rm -rf /work/ogre-src && mkdir -p /work/ogre-src
    tar -xjf /tmp/ogre.tar.bz2 -C /work/ogre-src --strip-components=1
fi

# 2. Windows fixes on top of the pinned source (--forward: skip if applied;
#    patch exits 1 when hunks are skipped, which is fine on a reused tree)
for p in /src/.github/workflows/ogre-win/patches/*.patch; do
    patch -s --forward -p1 -d /work/ogre-src < "$p" || true
done

# 3. Cross toolchain
cat > /toolchain.cmake <<'EOF'
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc-posix)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++-posix)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
EOF

# 4. Configure + build
cmake -S  /src/.github/workflows/ogre-win -B /work/build -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=/toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DOGRE_SRC=/work/ogre-src
cmake --build /work/build "$@"

