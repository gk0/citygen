#!/usr/bin/env python3
"""Generate CMake source lists from OGRE 1.6.5's shipped Code::Blocks projects.

The .cbp files are the authoritative MinGW file lists (see Ogre.workspace).
Usage: gen-sources.py <ogre-src-root> > ogre-sources.cmake
"""
import os
import sys
import xml.etree.ElementTree as ET

PROJECTS = [
    ("OGREMAIN_SOURCES", "OgreMain/scripts/OgreMain.cbp"),
    ("RENDERSYSTEM_GL_SOURCES", "RenderSystems/GL/scripts/RenderSystem_GL.cbp"),
    ("PLUGIN_OCTREE_SOURCES", "PlugIns/OctreeSceneManager/scripts/Plugin_OctreeSceneManager.cbp"),
    ("PLUGIN_CG_SOURCES", "PlugIns/CgProgramManager/scripts/Plugin_CgProgramManager.cbp"),
]
EXTS = (".cpp", ".c", ".rc")


def main():
    base = sys.argv[1]
    missing = []
    for var, cbp in PROJECTS:
        cbp_path = os.path.join(base, cbp)
        cbp_dir = os.path.dirname(cbp_path)
        files = []
        for unit in ET.parse(cbp_path).getiter("Unit") if False else ET.parse(cbp_path).iter("Unit"):
            fn = unit.get("filename", "")
            if not fn.lower().endswith(EXTS):
                continue
            p = os.path.normpath(os.path.join(cbp_dir, fn.replace("\\", "/")))
            rel = os.path.relpath(p, base).replace("\\", "/")
            if not os.path.exists(p):
                missing.append(rel)
                continue
            files.append(rel)
        files.sort()
        print(f"set({var}")
        for f in files:
            print(f"    ${{OGRE_SRC}}/{f}")
        print(")")
    if missing:
        print("MISSING FILES:", file=sys.stderr)
        for m in missing:
            print("  " + m, file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
