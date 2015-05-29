#--------------------------------------------------------------------
# Sigar Library
ivw_project(sigar)

#--------------------------------------------------------------------
# Define defintions
if(WIN32) 
	## make sure we only use the smallest set of 
	## headers on win32. Otherwise we get clashes 
	## between winsock2.h and winsock.h
	add_definitions(-DWIN32_LEAN_AND_MEAN)

	# force the correct version for the redist manifest
	#add_definitions(-D_BIND_TO_CURRENT_MFC_VERSION=1 -D_BIND_TO_CURRENT_CRT_VERSION=1)
    
    #disable useless warning
    add_definitions( "/wd4013 /wd4018 /wd4244 /wd4267 /wd4293 /wd4307 /wd4996 /nologo" )
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ivw_add_definition(IVW_SIGAR)
    
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

set(HEADER_FILES include/sigar.h 
	include/sigar_fileinfo.h 
	include/sigar_format.h 
	include/sigar_getline.h 
	include/sigar_log.h 
	include/sigar_private.h 
	include/sigar_ptql.h 
	include/sigar_util.h
)

include_directories(include/)

## linux
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(SIGAR_SRC src/os/linux/linux_sigar.c)

  include_directories(src/os/linux/)
endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")

## macosx, freebsd
if(CMAKE_SYSTEM_NAME MATCHES "(Darwin|FreeBSD)")
  set(SIGAR_SRC src/os/darwin/darwin_sigar.c)
  include_directories(src/os/darwin/)
  if(CMAKE_SYSTEM_NAME MATCHES "(Darwin)")
    ivw_include_directories(/Developer/Headers/FlatCarbon/)
    add_definitions(-DDARWIN)
    set(SIGAR_LINK_FLAGS "-framework CoreServices -framework IOKit")
  else(CMAKE_SYSTEM_NAME MATCHES "(Darwin)")
    ## freebsd needs libkvm
    set(SIGAR_LINK_FLAGS "-lkvm")
  endif(CMAKE_SYSTEM_NAME MATCHES "(Darwin)")
endif(CMAKE_SYSTEM_NAME MATCHES "(Darwin|FreeBSD)")

## solaris
if(CMAKE_SYSTEM_NAME MATCHES "(Solaris|SunOS)" )
  set(SIGAR_SRC
	src/os/solaris/solaris_sigar.c
	src/os/solaris/get_mib2.c
	src/os/solaris/kstats.c
	src/os/solaris/procfs.c
  )

  include_directories(src/os/solaris/)
  add_definitions(-DSOLARIS)
  set(SIGAR_LINK_FLAGS -lkstat -ldl -lnsl -lsocket -lresolv)
endif(CMAKE_SYSTEM_NAME MATCHES "(Solaris|SunOS)" )

## solaris
if(CMAKE_SYSTEM_NAME MATCHES "(hpux)" )
  set(SIGAR_SRC src/os/hpux/hpux_sigar.c)
  include_directories(src/os/hpux/)
  ivw_add_definition(-DSIGAR_HPUX)
  set(SIGAR_LINK_FLAGS -lnm)
endif(CMAKE_SYSTEM_NAME MATCHES "(hpux)" )

## aix
if(CMAKE_SYSTEM_NAME MATCHES "(AIX)" )
  set(SIGAR_SRC src/os/aix/aix_sigar.c)

  include_directories(src/os/aix/)
  set(SIGAR_LINK_FLAGS -lodm -lcfg)
endif(CMAKE_SYSTEM_NAME MATCHES "(AIX)" )

if(WIN32)
  if(BUILD_SHARED_LIBS)
    ivw_add_definition(SIGAR_SHARED)
    add_definitions(-DSIGAR_EXPORTS)
  endif()
  set(SIGAR_SRC src/os/win32/peb.c src/os/win32/win32_sigar.c src/os/win32/wmi.cpp)
  include_directories(src/os/win32)
else(WIN32)
      add_definitions(-DHAVE_CONFIG_H)
endif(WIN32)

set(SIGAR_SRC ${SIGAR_SRC}
  src/sigar.c
  src/sigar_cache.c
  src/sigar_fileinfo.c
  src/sigar_format.c
  src/sigar_getline.c
  src/sigar_ptql.c
  src/sigar_signal.c
  src/sigar_util.c
)

add_library(sigar ${SIGAR_SRC} ${HEADER_FILES})

if(WIN32)
	ivw_add_dependency_libraries(ws2_32 netapi32 version)
endif(WIN32)

if(SIGAR_LINK_FLAGS)
    ivw_add_link_flags(${SIGAR_LINK_FLAGS})
endif(SIGAR_LINK_FLAGS)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(sigar ext)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_set_cpack_name(core)
ivw_make_package(Sigar sigar)

if(CMAKE_SYSTEM_NAME MATCHES "(Darwin|FreeBSD)")
  #Ignore warnings in external packages
  set_target_properties(sigar PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_INHIBIT_ALL_WARNINGS YES)
endif(CMAKE_SYSTEM_NAME MATCHES "(Darwin|FreeBSD)")

