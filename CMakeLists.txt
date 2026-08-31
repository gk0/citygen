# citygen — wxWidgets + OGRE 1.6.5 city generator
# Linux build (GCC). OGRE 1.6.5 must be built & installed first (default prefix: ~/ogre165).
cmake_minimum_required(VERSION 3.10)
project(citygen CXX)

set(CMAKE_CXX_STANDARD 11)          # wx3.2 headers require C++11; code is C++98-compatible
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # clangd / clang tooling need this

# debug symbols for gdb/clangd breakpoints, -O2 so the renderer stays usable
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

# --- OGRE 1.6.5 (autotools install prefix) ---
set(OGRE_HOME "$ENV{HOME}/ogre165" CACHE PATH "OGRE 1.6.5 install prefix")

# --- vendored tinyxml (2.6.2, zlib licence) ---
file(GLOB TINYXML_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyxml/*.cpp)
add_library(tinyxml_static STATIC ${TINYXML_SOURCES})
target_include_directories(tinyxml_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyxml)
target_compile_definitions(tinyxml_static PRIVATE TIXML_USE_STL)

# --- citygen sources (from citygen.vcproj; stdafx.cpp is MSVC PCH boilerplate) ---
file(GLOB CITYGEN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(REMOVE_ITEM CITYGEN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ogre-smoke-test.cpp)   # standalone test, has its own main()

add_executable(citygen ${CITYGEN_SOURCES})

target_include_directories(citygen PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OGRE_HOME}/include/OGRE
)

# 2009-era code on a 2025 toolchain:
#  - OGRE_MEMORY_ALLOCATOR=STD must match how libOgreMain was configured (--with-allocator=std)
#  - forced C headers: old code relies on transitive includes modern libstdc++ dropped
target_compile_definitions(citygen PRIVATE
    OGRE_MEMORY_ALLOCATOR=OGRE_MEMORY_ALLOCATOR_STD
    TIXML_USE_STL
    USE_XPM_BITMAPS
)
target_compile_options(citygen PRIVATE
    "SHELL:-include stddef.h" "SHELL:-include string.h" "SHELL:-include limits.h"
    "SHELL:-include stdlib.h" "SHELL:-include stdio.h"
    # -Wno-write-strings: ~1445 warnings, almost all from bitmaps/*.xpm icon data
    # -Wno-deprecated-declarations: std::binary_function (23) + gtk_widget_set_double_buffered (1)
    -Wno-write-strings -Wno-deprecated-declarations
)

# --- wxWidgets (3.2: aui + propgrid are core libs) ---
find_package(wxWidgets REQUIRED COMPONENTS core base aui propgrid)
include(${wxWidgets_USE_FILE})
target_link_libraries(citygen PRIVATE ${wxWidgets_LIBRARIES})

# wx3.2 uses the GTK3 backend; WorldFrame.cpp pulls in gdk/gtk headers directly
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
target_include_directories(citygen PRIVATE ${GTK3_INCLUDE_DIRS})
target_link_libraries(citygen PRIVATE ${GTK3_LIBRARIES})

# --- OGRE ---
find_library(OGREMAIN_LIBRARY NAMES OgreMain PATHS ${OGRE_HOME}/lib NO_DEFAULT_PATH REQUIRED)

# ColladaDoc.cpp uses boost::filesystem (compiled lib, not header-only).
# Only the runtime .so is installed (no -dev package / CMake config), so link it
# directly by soname; boost::system is header-only since Boost 1.69.
find_library(BOOST_FILESYSTEM_LIBRARY
    NAMES boost_filesystem libboost_filesystem.so.1.83.0
    REQUIRED)
target_link_libraries(citygen PRIVATE ${OGREMAIN_LIBRARY} tinyxml_static
    ${BOOST_FILESYSTEM_LIBRARY} GL X11)

# --- runtime dir: binary + media + the OGRE plugins citygen loads ---
set(RUNTIME_DIR ${CMAKE_CURRENT_BINARY_DIR}/runtime)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Media DESTINATION ${RUNTIME_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/bitmaps DESTINATION ${RUNTIME_DIR})

set(RUNTIME_OGRE_PLUGINS
    ${OGRE_HOME}/lib/OGRE/RenderSystem_GL.so
    ${OGRE_HOME}/lib/OGRE/Plugin_OctreeSceneManager.so
)
# Optional: present when OGRE was configured with the Cg toolkit
# (required for the normal/relief mapping shaders)
if(EXISTS ${OGRE_HOME}/lib/OGRE/Plugin_CgProgramManager.so)
    list(APPEND RUNTIME_OGRE_PLUGINS ${OGRE_HOME}/lib/OGRE/Plugin_CgProgramManager.so)
endif()

add_custom_command(TARGET citygen POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:citygen> ${RUNTIME_DIR}/
    COMMENT "Staging runtime dir with media and OGRE plugins"
)
foreach(PLUGIN ${RUNTIME_OGRE_PLUGINS})
    add_custom_command(TARGET citygen POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PLUGIN} ${RUNTIME_DIR}/)
endforeach()
