#
# project: darkstar
#
cmake_minimum_required(VERSION 2.8)

# include the fips main cmake file
get_filename_component( FIPS_ROOT_DIR "/Users/joeld/Stuff/oryol-test/fips" ABSOLUTE)
MESSAGE( STATUS "Fips root dir: " ${FIPS_ROOT_DIR} )

include ("${FIPS_ROOT_DIR}/cmake/fips.cmake")

fips_setup()
fips_project(darkstar)
fips_add_subdirectory(code)

# manually define imports, don't bring all of Oryol
fips_ide_group("Oryol")
fips_import_oryol_Core()
fips_import_oryol_IO()
fips_import_oryol_LocalFS()
#fips_import_oryol_Messaging()
fips_import_oryol_Gfx()
fips_import_oryol_Assets()
fips_import_oryol_Resource()

fips_ide_group("Libs")
fips_import_fips_glfw_glfw3()
#fips_import_oryol_flextgl()
#fips_import_oryol_ConvertUTF()

fips_ide_group("")

fips_finish()