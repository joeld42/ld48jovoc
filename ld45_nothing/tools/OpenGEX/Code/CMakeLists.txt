# add_definitions(-DOGEX_EXAMPLE_MAIN)
fips_begin_module(OpenGEX)
    fips_files(
		OpenGEX.cpp
		OpenGEX.h
		)
    fips_deps(OpenDDL)
fips_end_module()
