#
# project: Darkstar (LDJAM Basecode)
#
cmake_minimum_required(VERSION 2.8)

#set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")
set(MACOSX_DEPLOYMENT_TARGET 10.12)


# include the fips main cmake file
get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

# this hack is required for turbobadger since we need to make
# sure that our overriden config.h is included
#include_directories(../oryol-tbui/src/TBUI/tb)

# force ORYOL_SAMPLE_URL to location of oryol-samples webpage
#set(ORYOL_SAMPLE_URL "http://floooh.github.com/oryol-samples/data/" CACHE STRING "Samples URL" FORCE)

set(MACOSX_DEPLOYMENT_TARGET 10.12)
set(FIPS_EMSCRIPTEN_RELATIVE_SHELL_HTML "html/shell_darkstar.html")

fips_setup()
fips_project(darkstar)
include_directories(code)


# FIXME: only add tools directory to tools build
fips_include_directories(tools/OpenDDL/Code)
fips_include_directories(tools/OpenGEX/Code)
fips_include_directories(tools/ldjamfile)

fips_add_subdirectory(code)
fips_add_subdirectory(tools)

# Don't need these because auto_import is on in fips.yml
# fips_ide_group("Oryol")
# fips_import_oryol_Core()
# fips_import_oryol_Gfx()
# fips_import_oryol_IO()
# fips_import_oryol_LocalFS()
# fips_import_oryol_Assets()
# fips_import_oryol_Resource()
# fips_import_oryol_Input()
# fips_ide_group("Libs")
# fips_import_fips_glfw_glfw3()
# fips_import_oryol_nuklear_NKUI()
# fips_import_oryol_imgui_IMUI()
# fips_ide_group("")

fips_finish()


