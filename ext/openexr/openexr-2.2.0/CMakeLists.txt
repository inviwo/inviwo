#CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ivw_project(openexr)

find_package(openexr-iex REQUIRED)
find_package(openexr-iexmath REQUIRED)
find_package(openexr-imath REQUIRED)
find_package(openexr-half REQUIRED)
find_package(openexr-ilmthread REQUIRED)

SET(OPENEXR_VERSION_MAJOR "2")
SET(OPENEXR_VERSION_MINOR "2")
SET(OPENEXR_VERSION_PATCH "0")

SET(OPENEXR_VERSION ${OPENEXR_VERSION_MAJOR}.${OPENEXR_VERSION_MINOR}.${OPENEXR_VERSION_PATCH})
SET(OPENEXR_VERSION_API ${OPENEXR_VERSION_MAJOR}_${OPENEXR_VERSION_MINOR})


# enable the tests
# ENABLE_TESTING()


# distro building 
# SET(CPACK_PACKAGE_VERSION_MAJOR "${OPENEXR_VERSION_MAJOR}")
# SET(CPACK_PACKAGE_VERSION_MINOR "${OPENEXR_VERSION_MINOR}")
# SET(CPACK_PACKAGE_VERSION_PATCH "${OPENEXR_VERSION_PATCH}")
# SET(CPACK_SOURCE_GENERATOR "TGZ")
# set(CPACK_SOURCE_PACKAGE_FILE_NAME
#     "${CMAKE_PROJECT_NAME}-${OPENEXR_VERSION}"
#   )
# set(CPACK_SOURCE_IGNORE_FILES
#   "/.git*;/.cvs*;${CPACK_SOURCE_IGNORE_FILES}")
# INCLUDE ( CPack )



# Allow the developer to select if Dynamic or Static libraries are built
# OPTION (BUILD_SHARED_LIBS    "Build Shared Libraries" ON)
# OPTION (USE_ZLIB_WINAPI      "Use ZLib Win API"       OFF)
OPTION (NAMESPACE_VERSIONING "Use Namespace Versioning" ON)

# Setup osx rpathing
# SET (CMAKE_MACOSX_RPATH 1)
# SET (BUILD_WITH_INSTALL_RPATH 1)

ADD_DEFINITIONS ( -DHAVE_CONFIG_H )

INCLUDE_DIRECTORIES (
  ${CMAKE_BINARY_DIR}/modules/_generated/ext/openexr
  IlmImf
  IlmImfUtil
  # exrmaketiled
  # exrenvmap
  # exrmakepreview
  # exrmultiview
  # IlmImfFuzzTest
)

FIND_PACKAGE(ZLIB REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

IF (NOT WIN32)
  SET ( PTHREAD_LIB pthread )
ENDIF()

## include ilm directories
INCLUDE_DIRECTORIES(${OPENEXR-HALF_INCLUDE_DIR}
  ${OPENEXR-IEX_INCLUDE_DIR}
  ${OPENEXR-IEXMATH_INCLUDE_DIR}
  ${OPENEXR-IMATH_INCLUDE_DIR}
  ${OPENEXR-ILMTHREAD_INCLUDE_DIR}
)

# INCLUDE_DIRECTORIES ( ${ILMBASE_PACKAGE_PREFIX}/include/OpenEXR )
# LINK_DIRECTORIES ( ${ILMBASE_PACKAGE_PREFIX}/lib )
# MESSAGE ( "ILMBASE_PACKAGE_PREFIX = " ${ILMBASE_PACKAGE_PREFIX})

SET (LIB_TYPE STATIC)
IF (BUILD_SHARED_LIBS)
  # User wants to build Dynamic Libraries, so change the LIB_TYPE variable to CMake keyword 'SHARED'
  SET (LIB_TYPE SHARED)
    IF (WIN32)
    ADD_DEFINITIONS(-DOPENEXR_DLL)
  ENDIF ()
ENDIF ()

IF (USE_ZLIB_WINAPI)
  ADD_DEFINITIONS(-DZLIB_WINAPI)
ENDIF ()

# Test for GCC-style inline asm support with AVX instructions
INCLUDE (CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES (
	"
	int main()
	{
	 #if defined(__GNUC__) && defined(__SSE2__) 
		 int n   = 0;
		 int eax = 0;
		 int edx = 0;
		 __asm__(
			 \"xgetbv     ;\"
			 \"vzeroupper  \"
			 : \"=a\"(eax), \"=d\"(edx) : \"c\"(n) : );
	 #else
		 #error No GCC style inline asm supported for AVX instructions
	 #endif
	}
	" HAVE_GCC_INLINE_ASM_AVX)

# Check if sysconf(_SC_NPROCESSORS_ONLN) can be used for CPU count
CHECK_CXX_SOURCE_COMPILES (
    "
    #include <unistd.h>
    int main()
    {
        sysconf(_SC_NPROCESSORS_ONLN);
    }
    " HAVE_SYSCONF_NPROCESSORS_ONLN)


##########################
# OpenEXRConfig.h generation
##########################

set(openexrConfigFile ${CMAKE_BINARY_DIR}/modules/_generated/ext/openexr/OpenEXRConfig.h)
set(tmpOpenexrConfigFile ${openexrConfigFile}.tmp)

IF (WIN32)
  FILE ( WRITE ${tmpOpenexrConfigFile} "#define OPENEXR_IMF_HAVE_COMPLETE_IOMANIP 1\n" )
ELSEIF (APPLE)
  FILE ( WRITE ${tmpOpenexrConfigFile} "#define OPENEXR_IMF_HAVE_DARWIN 1\n" )
  FILE ( APPEND ${tmpOpenexrConfigFile} "#define OPENEXR_IMF_HAVE_COMPLETE_IOMANIP 1\n" )
  FILE ( APPEND ${tmpOpenexrConfigFile} "#include <string.h>\n" )
ELSE ()
  # Linux
  FILE ( WRITE ${tmpOpenexrConfigFile} "#define OPENEXR_IMF_HAVE_LINUX_PROCFS 1\n" )
  FILE ( APPEND ${tmpOpenexrConfigFile} "#define OPENEXR_IMF_HAVE_COMPLETE_IOMANIP 1\n" )
  FILE ( APPEND ${tmpOpenexrConfigFile} "#define OPENEXR_IMF_HAVE_LARGE_STACK 1\n" )
ENDIF()
  
IF (NAMESPACE_VERSIONING)
    FILE ( APPEND ${tmpOpenexrConfigFile} "#define OPENEXR_IMF_INTERNAL_NAMESPACE_CUSTOM 1\n")
    FILE ( APPEND ${tmpOpenexrConfigFile} "#define OPENEXR_IMF_NAMESPACE Imf\n" )
    FILE ( APPEND ${tmpOpenexrConfigFile} "#define OPENEXR_IMF_INTERNAL_NAMESPACE Imf_${OPENEXR_VERSION_API}\n\n" )
ELSE ()
    FILE ( APPEND ${tmpOpenexrConfigFile} "#define OPENEXR_IMF_INTERNAL_NAMESPACE_CUSTOM 0\n")
    FILE ( APPEND ${tmpOpenexrConfigFile} "#define OPENEXR_IMF_NAMESPACE Imf\n" )
    FILE ( APPEND ${tmpOpenexrConfigFile} "#define OPENEXR_IMF_INTERNAL_NAMESPACE Imf\n\n" )
ENDIF ()

FILE ( APPEND ${tmpOpenexrConfigFile} "#define OPENEXR_VERSION_STRING \"${OPENEXR_VERSION}\"\n" )
FILE ( APPEND ${tmpOpenexrConfigFile} "#define OPENEXR_PACKAGE_STRING \"OpenEXR ${OPENEXR_VERSION}\"\n" )


FILE ( APPEND ${tmpOpenexrConfigFile} "
#define OPENEXR_VERSION_MAJOR ${OPENEXR_VERSION_MAJOR}
#define OPENEXR_VERSION_MINOR ${OPENEXR_VERSION_MINOR}
#define OPENEXR_VERSION_PATCH ${OPENEXR_VERSION_PATCH}
")


  FILE ( APPEND ${tmpOpenexrConfigFile} "
// Version as a single hex number, e.g. 0x01000300 == 1.0.3
#define OPENEXR_VERSION_HEX ((OPENEXR_VERSION_MAJOR << 24) | \\
                             (OPENEXR_VERSION_MINOR << 16) | \\
                             (OPENEXR_VERSION_PATCH <<  8))\n
")

IF (HAVE_GCC_INLINE_ASM_AVX)
  FILE ( APPEND ${tmpOpenexrConfigFile} "#define OPENEXR_IMF_HAVE_GCC_INLINE_ASM_AVX 1\n" )
ENDIF()
  
IF (HAVE_SYSCONF_NPROCESSORS_ONLN)
  FILE ( APPEND ${tmpOpenexrConfigFile} "#define OPENEXR_IMF_HAVE_SYSCONF_NPROCESSORS_ONLN 1\n" )
ENDIF()
  
## check if the newly created temporary config file is different from the existing one 
## and overwrite it if necessary
##
## use md5 checksums to check for differences
if (EXISTS ${openexrConfigFile})
    file(MD5 ${openexrConfigFile} openexrConfigMD5)
else()
    set(openexrConfigMD5 "")
endif()
file(MD5 ${tmpOpenexrConfigFile} tmpOpenexrConfigMD5)

if(NOT "${openexrConfigMD5}" STREQUAL "${tmpOpenexrConfigMD5}")
    file(RENAME ${tmpOpenexrConfigFile} ${openexrConfigFile})
else()
    file(REMOVE ${tmpOpenexrConfigFile})
endif()


SET (OPENEXR_LIBSUFFIX "")
SET (ILMBASE_LIBSUFFIX "")
IF (NAMESPACE_VERSIONING)
  SET ( OPENEXR_LIBSUFFIX "-${OPENEXR_VERSION_API}" )
  # assume same NAMESPACE_VERSION setting for IlmBase for now
  SET ( ILMBASE_LIBSUFFIX "-${OPENEXR_VERSION_API}" )
ENDIF ()

##########################
# IlmImf library
##########################
ADD_SUBDIRECTORY ( IlmImf )

SET_TARGET_PROPERTIES ( openexr-ilmimf
  PROPERTIES
  VERSION 22.0.0
  SOVERSION 22
  # OUTPUT_NAME "IlmImf${OPENEXR_LIBSUFFIX}"
  )
  
##########################
# IlmImfUtil library
##########################
ADD_SUBDIRECTORY ( IlmImfUtil )

SET_TARGET_PROPERTIES ( openexr-ilmimfutil
  PROPERTIES
  VERSION 22.0.0
  SOVERSION 22
  # OUTPUT_NAME "IlmImfUtil${OPENEXR_LIBSUFFIX}"
  )
  
##########################
# Example Code
##########################
# ADD_SUBDIRECTORY ( IlmImfExamples )

  
##########################
# Tests
##########################
# ADD_SUBDIRECTORY ( IlmImfTest )
# ADD_SUBDIRECTORY ( IlmImfUtilTest )
# ADD_SUBDIRECTORY ( IlmImfFuzzTest )


##########################
# Binaries / Utilities
##########################
# ADD_SUBDIRECTORY ( exrheader )
# ADD_SUBDIRECTORY ( exrmaketiled )
# ADD_SUBDIRECTORY ( exrstdattr )
# ADD_SUBDIRECTORY ( exrmakepreview )
# ADD_SUBDIRECTORY ( exrenvmap )
# ADD_SUBDIRECTORY ( exrmultiview )
# ADD_SUBDIRECTORY ( exrmultipart )


##########################
# Installation
##########################

# INSTALL ( FILES
#   ${CMAKE_BINARY_DIR}/modules/_generated/ext/openexr/OpenEXRConfig.h
#   DESTINATION
#   ${CMAKE_INSTALL_PREFIX}/include/OpenEXR
#   )

# # Documentation
# INSTALL ( FILES
#   doc/TechnicalIntroduction.pdf
#   doc/ReadingAndWritingImageFiles.pdf
#   doc/OpenEXRFileLayout.pdf
#   doc/MultiViewOpenEXR.pdf
#   doc/InterpretingDeepPixels.pdf
#   doc/TheoryDeepPixels.pdf
#   DESTINATION
#   ${CMAKE_INSTALL_PREFIX}/share/doc/OpenEXR-${OPENEXR_VERSION}
#   )

# # Examples
# INSTALL ( FILES
#   IlmImfExamples/main.cpp
#   IlmImfExamples/drawImage.cpp
#   IlmImfExamples/rgbaInterfaceExamples.cpp
#   IlmImfExamples/rgbaInterfaceTiledExamples.cpp
#   IlmImfExamples/generalInterfaceExamples.cpp
#   IlmImfExamples/lowLevelIoExamples.cpp
#   IlmImfExamples/previewImageExamples.cpp
#   IlmImfExamples/generalInterfaceTiledExamples.cpp
#   IlmImfExamples/generalInterfaceTiledExamples.h
#   IlmImfExamples/drawImage.h
#   IlmImfExamples/rgbaInterfaceExamples.h
#   IlmImfExamples/generalInterfaceExamples.h
#   IlmImfExamples/rgbaInterfaceTiledExamples.h
#   IlmImfExamples/lowLevelIoExamples.h
#   IlmImfExamples/previewImageExamples.h
#   IlmImfExamples/namespaceAlias.h
#   DESTINATION
#   ${CMAKE_INSTALL_PREFIX}/share/doc/OpenEXR-${OPENEXR_VERSION}/examples
#   )
