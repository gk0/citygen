#!/bin/sh
# Generate CMake source lists from OGRE 1.6.5's shipped Code::Blocks projects.
#
# The .cbp files are the authoritative MinGW file lists (see Ogre.workspace);
# this replaces the MSVC/MinGW toolchain assumptions with plain relative paths
# usable from the CMake port in this directory.
#
# Usage: gen-sources.sh <ogre-src-root> > ogre-sources.cmake
set -eu

BASE="${1:?usage: gen-sources.sh <ogre-src-root>}"

emit() {
    var="$1"
    cbp="$2"
    dir=$(dirname "$BASE/$cbp")
    echo "set($var"
    grep '<Unit' "$BASE/$cbp" \
        | grep -o 'filename="[^"]*"' \
        | sed 's/^filename="//; s/"$//' \
        | grep -iE '\.(cpp|c|rc)$' \
        | tr '\\' '/' \
        | LC_ALL=C sort -u \
        | while read -r f; do
            [ -f "$dir/$f" ] || { echo "MISSING: $dir/$f" >&2; exit 1; }
            rel=$(realpath --relative-to="$BASE" "$dir/$f")
            echo "    \${OGRE_SRC}/$rel"
          done
    echo ")"
}

emit OGREMAIN_SOURCES OgreMain/scripts/OgreMain.cbp
emit RENDERSYSTEM_GL_SOURCES RenderSystems/GL/scripts/RenderSystem_GL.cbp
emit PLUGIN_OCTREE_SOURCES PlugIns/OctreeSceneManager/scripts/Plugin_OctreeSceneManager.cbp
emit PLUGIN_CG_SOURCES PlugIns/CgProgramManager/scripts/Plugin_CgProgramManager.cbp
