#CMAKE_MINIMUM_REQUIRED(VERSION 2.8)


OPTION(OpenEXR_GENERATE_LUT "Enable the generation of lookup tables used by OpenEXR (pre-computed LuTs will be used if disabled)" OFF)

IF(${generate_lookup_tables})
  ## create lookup tables in the generated files folder of the build
  SET(lookup_table_dir ${CMAKE_BINARY_DIR}/modules/_generated/ext/openexr)
ELSE()
  ## use the pre-computed lookup tables
  SET(lookup_table_dir ${CMAKE_CURRENT_SOURCE_DIR}/lut)
ENDIF()

ADD_SUBDIRECTORY(ilmbase-2.2.0)
ADD_SUBDIRECTORY(openexr-2.2.0)

