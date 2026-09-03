# syntax=docker/dockerfile:1
###############################################################################
# Reproducible build for citygen (wxWidgets 3.2 + OGRE 1.6.5 + Cg shaders)
#
# Targets:
#   build — installs deps, builds OGRE 1.6.5 from the pinned official tarball,
#           builds citygen, stages a self-contained runtime in /out/runtime
#   smoke — build + headless run of citygen under Xvfb/llvmpipe, checks the
#           Ogre log for the Cg plugin and clean script parsing
#
# Local:  docker build --target build -t citygen-build .
#         docker build --target smoke -t citygen-smoke .
# CI:     .github/workflows/ci.yml (same targets, GHA layer cache)
###############################################################################

FROM debian:trixie AS build

ARG OGRE_URL="https://downloads.sourceforge.net/project/ogre/ogre/1.6.5/ogre-v1-6-5.tar.bz2"
ARG OGRE_SHA256="7fc0e948679c1c1f10751756d267a41d0e3395a6520a23f7853a0ae39a1281f5"
ENV OGRE_PREFIX=/opt/ogre165

# trixie: enable non-free for nvidia-cg-toolkit (Cg Program Manager plugin)
RUN sed -i 's/^Components:.*/Components: main contrib non-free non-free-firmware/' \
        /etc/apt/sources.list.d/debian.sources \
 && apt-get update \
 && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        pkg-config \
        curl \
        bzip2 \
        ca-certificates \
        libfreetype-dev \
        libzzip-dev \
        libfreeimage-dev \
        libboost-dev \
        libx11-dev \
        libxext-dev \
        libxrandr-dev \
        libxt-dev \
        libxaw7-dev \
        libxxf86vm-dev \
        libgl1-mesa-dev \
        libglu1-mesa-dev \
        nvidia-cg-toolkit \
        libwxgtk3.2-dev \
        libgtk-3-dev \
        libboost-filesystem-dev \
        libboost-system-dev \
        xvfb \
        libgl1-mesa-dri \
        libglx-mesa0 \
 && rm -rf /var/lib/apt/lists/*

# Ogre 1.6.5 detects FreeType exclusively via freetype-config, which modern
# Debian no longer ships - shim it onto pkg-config
RUN printf '#!/bin/sh\ncase "$1" in\n  --version) exec pkg-config --modversion freetype2 ;;\n  *) exec pkg-config "$@" freetype2 ;;\nesac\n' \
        > /usr/local/bin/freetype-config \
 && chmod +x /usr/local/bin/freetype-config

# --- OGRE 1.6.5 (autotools, pinned tarball, std allocator to match citygen) ---
RUN curl -fsSL -o /tmp/ogre.tar.bz2 "${OGRE_URL}" \
 && echo "${OGRE_SHA256}  /tmp/ogre.tar.bz2" | sha256sum -c - \
 && mkdir /tmp/ogre-src \
 && tar -xjf /tmp/ogre.tar.bz2 -C /tmp/ogre-src --strip-components=1 \
 && cd /tmp/ogre-src \
 && ./configure --prefix="${OGRE_PREFIX}" --with-allocator=std --disable-ogre-demos \
        CXXFLAGS="-O2 -std=gnu++98 -include stddef.h -DXTSTRINGDEFINES" \
 && make -j"$(nproc)" \
 && make install \
 && rm -rf /tmp/ogre-src /tmp/ogre.tar.bz2

# --- citygen ---
WORKDIR /src
COPY . /src
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DOGRE_HOME="${OGRE_PREFIX}" \
 && cmake --build build -j"$(nproc)"

# --- self-contained runtime: binary + media + bitmaps + OGRE plugins ---
RUN mkdir -p /out && cp -a build/runtime /out/runtime

###############################################################################
# Headless smoke test: run citygen under Xvfb (llvmpipe), then verify the
# Ogre log shows the Cg plugin installed and resources parsed cleanly.
###############################################################################
FROM build AS smoke
# xvfb-run needs xauth, which is only a Recommends of xvfb and thus absent
# from the --no-install-recommends build stage; install it here so the
# cached build layers are not invalidated
RUN apt-get update \
 && apt-get install -y --no-install-recommends xauth \
 && rm -rf /var/lib/apt/lists/* \
 && bash /src/.github/workflows/smoke-test.sh /out/runtime
