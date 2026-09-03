# CMake port of OGRE 1.6.5 for MinGW Windows builds.
#
# OGRE 1.6.5 shipped no Windows build system beyond VS2005/2008 solutions and
# Code::Blocks projects (Ogre.workspace). This file rebuilds the subset citygen
# needs - OgreMain, RenderSystem_GL, Plugin_OctreeSceneManager (which includes
# the TerrainSceneManager) and Plugin_CgProgramManager - using the source file
# lists and flags from those shipped Code::Blocks projects (see gen-sources.py).
#
# Inputs:
#   OGRE_SRC      OGRE 1.6.5 source root (pinned tarball, see Dockerfile)
#   MSYS2_PREFIX  MSYS2 mingw64 prefix (freeimage/zziplib/zlib/freetype)
#   CG_PREFIX     NVIDIA Cg SDK extracted with innoextract (include/, lib.x64/)
#
# Memory allocator is STD to match the Linux/Docker build
# (--with-allocator=std); nedmalloc is not compiled.
cmake_minimum_required(VERSION 3.16)
project(Ogre165Win LANGUAGES CXX C RC)

set(OGRE_SRC "" CACHE PATH "OGRE 1.6.5 source root")
set(MSYS2_PREFIX "/opt/msys2/mingw64" CACHE PATH "MSYS2 mingw64 prefix")
set(CG_PREFIX "/opt/cg/app" CACHE PATH "extracted NVIDIA Cg SDK prefix")

# Source lists are generated from OGRE's shipped Code::Blocks projects (the
# authoritative MinGW file lists); derived from the pinned OGRE_SRC tree, so
# not committed.
find_program(GEN_SH sh REQUIRED)
set(OGRE_SOURCES_CMAKE "${CMAKE_CURRENT_BINARY_DIR}/ogre-sources.cmake")
execute_process(
    COMMAND "${GEN_SH}" "${CMAKE_CURRENT_LIST_DIR}/gen-sources.sh" "${OGRE_SRC}"
    OUTPUT_FILE "${OGRE_SOURCES_CMAKE}"
    RESULT_VARIABLE GEN_RES
    ERROR_VARIABLE GEN_ERR)
if(NOT GEN_RES EQUAL 0)
    message(FATAL_ERROR "gen-sources.sh failed: ${GEN_ERR}")
endif()
include("${OGRE_SOURCES_CMAKE}")

# Modern libstdc++ dropped transitive includes the 2009 code relies on
# (same forced-include trick as the Linux build in the top-level CMakeLists).
add_compile_options(
    "SHELL:-include stddef.h" "SHELL:-include string.h" "SHELL:-include limits.h"
    "SHELL:-include stdlib.h" "SHELL:-include stdio.h")

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Upstream naming (OgreMain.dll, RenderSystem_GL.dll, ...): citygen loads
# plugins by name via Root::loadPlugin, and Windows LoadLibrary just appends
# ".dll" - MinGW's default "lib" prefix would break that.
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_IMPORT_LIBRARY_PREFIX "")

# Common defines from the Code::Blocks projects; per-target export defines
# control OgreMain's dllimport/dllexport macros.
set(OGRE_DEFS WIN32 _WINDOWS _USRDLL _MT
    OGRE_MEMORY_ALLOCATOR=OGRE_MEMORY_ALLOCATOR_STD
    $<$<CONFIG:Debug>:_DEBUG> $<$<CONFIG:Release>:NDEBUG>)

# ---------------------------------------------------------------- OgreMain ---
add_library(OgreMain SHARED ${OGREMAIN_SOURCES})
target_include_directories(OgreMain PRIVATE
    ${OGRE_SRC}/OgreMain/include
    ${OGRE_SRC}/OgreMain/include/WIN32
    ${OGRE_SRC}/OgreMain/src
    ${MSYS2_PREFIX}/include
    ${MSYS2_PREFIX}/include/freetype2)
target_compile_definitions(OgreMain PRIVATE OGRE_NONCLIENT_BUILD ${OGRE_DEFS})
target_link_libraries(OgreMain PRIVATE
    ${MSYS2_PREFIX}/lib/libfreeimage.dll.a
    ${MSYS2_PREFIX}/lib/libfreetype.dll.a
    ${MSYS2_PREFIX}/lib/libzzip.dll.a
    ${MSYS2_PREFIX}/lib/libz.dll.a
    kernel32 user32 gdi32 advapi32)

# --------------------------------------------------------- RenderSystem_GL ---
add_library(RenderSystem_GL SHARED ${RENDERSYSTEM_GL_SOURCES})
target_include_directories(RenderSystem_GL PRIVATE
    ${OGRE_SRC}/OgreMain/include
    ${OGRE_SRC}/RenderSystems/GL/include
    ${OGRE_SRC}/RenderSystems/GL/src
    ${OGRE_SRC}/RenderSystems/GL/src/GLSL/include
    ${OGRE_SRC}/RenderSystems/GL/src/atifs/include
    ${OGRE_SRC}/RenderSystems/GL/src/nvparse
    ${OGRE_SRC}/RenderSystems/GL/src/nvparse/winheaders
    ${OGRE_SRC}/RenderSystems/GL/src/win32
    ${MSYS2_PREFIX}/include)
target_compile_definitions(RenderSystem_GL PRIVATE
    OGRE_GLPLUGIN_EXPORTS OGRE_GL_USE_MULTITEXTURING ${OGRE_DEFS})
target_link_libraries(RenderSystem_GL PRIVATE OgreMain opengl32 glu32 gdi32)

# ------------------------------------------------- Plugin_OctreeSceneManager ---
# Also provides the TerrainSceneManager citygen instantiates by name.
add_library(Plugin_OctreeSceneManager SHARED ${PLUGIN_OCTREE_SOURCES})
target_include_directories(Plugin_OctreeSceneManager PRIVATE
    ${OGRE_SRC}/OgreMain/include
    ${OGRE_SRC}/PlugIns/OctreeSceneManager/include)
target_compile_definitions(Plugin_OctreeSceneManager PRIVATE
    OGRE_TERRAINPLUGIN_EXPORTS ${OGRE_DEFS})
target_link_libraries(Plugin_OctreeSceneManager PRIVATE OgreMain)

# -------------------------------------------------- Plugin_CgProgramManager ---
add_library(Plugin_CgProgramManager SHARED ${PLUGIN_CG_SOURCES})
target_include_directories(Plugin_CgProgramManager PRIVATE
    ${OGRE_SRC}/OgreMain/include
    ${OGRE_SRC}/PlugIns/CgProgramManager/include
    ${CG_PREFIX}/include)
target_compile_definitions(Plugin_CgProgramManager PRIVATE
    OGRE_CGPLUGIN_EXPORTS ${OGRE_DEFS})
target_link_libraries(Plugin_CgProgramManager PRIVATE OgreMain
    ${CG_PREFIX}/lib.x64/cg.lib
    ${CG_PREFIX}/lib.x64/cgGL.lib)
