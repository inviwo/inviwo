# Special CMake Module required when doing Java Testing
if(BUILD_TESTING AND GDCM_WRAP_JAVA)
  include(${GDCM_SOURCE_DIR}/CMake/UseJavaTest.cmake)
endif()

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
option(GDCM_AUTOLOAD_GDCMJNI "Automatically load gdcmjni" ON)
option(GDCM_JAVA_INCLUDE_SRC "Add source (*.java) to jar file" OFF)
option(GDCM_BUILD_FULL_JAVA_JAR "Build gdcm jar with encapsulated gdcmjni" OFF)
mark_as_advanced(GDCM_AUTOLOAD_GDCMJNI GDCM_JAVA_INCLUDE_SRC GDCM_BUILD_FULL_JAVA_JAR)

include_directories(
  "${GDCM_BINARY_DIR}/Source/Common"
  "${GDCM_SOURCE_DIR}/Source/Common"
  "${GDCM_SOURCE_DIR}/Source/DataStructureAndEncodingDefinition"
  "${GDCM_SOURCE_DIR}/Source/InformationObjectDefinition"
  "${GDCM_SOURCE_DIR}/Source/MediaStorageAndFileFormat"
  "${GDCM_SOURCE_DIR}/Source/DataDictionary"
  "${GDCM_SOURCE_DIR}/Source/MessageExchangeDefinition"
)

# $ export JAVA_HOME=/usr/lib/j2sdk1.6-sun/
# $ export JAVA_HOME=/usr/lib/jvm/java-1.5.0-sun/
find_package(Java ${GDCM_DEFAULT_JAVA_VERSION} REQUIRED) # javac, jar
find_package(JNI REQUIRED)
include_directories(
  ${JNI_INCLUDE_DIRS}
  )
set_source_files_properties(gdcm.i PROPERTIES CPLUSPLUS ON)

set(GDCM_JAVA_HOME ${CMAKE_CURRENT_BINARY_DIR}/java/gdcm)
file(MAKE_DIRECTORY ${GDCM_JAVA_HOME})
set(CMAKE_SWIG_OUTDIR "${GDCM_JAVA_HOME}")

set(CMAKE_SWIG_FLAGS "-package gdcm")
separate_arguments(CMAKE_SWIG_FLAGS)
# http://www.debian.org/doc/packaging-manuals/java-policy/x105.html
SWIG_ADD_MODULE(gdcmjni java gdcm.i)
SWIG_LINK_LIBRARIES(gdcmjni gdcmMSFF
  gdcmMEXD
  # There is no real point in linking explicitly to jni libraries. This is a java module
  # it could contains undefined symbols after all.
  #${JNI_LIBRARIES}
)
set_target_properties(${SWIG_MODULE_gdcmjni_REAL_NAME} PROPERTIES LINK_INTERFACE_LIBRARIES "")
if (WIN32)
  # disable lib prefix on windows with mingw
  set_target_properties(${SWIG_MODULE_gdcmjni_REAL_NAME} PROPERTIES PREFIX "")
endif()
set_property(TARGET ${SWIG_MODULE_gdcmjni_REAL_NAME} PROPERTY NO_SONAME 1)

# swig-java dummy run:
# the following execute_process is a dummy run and generate all *.java files in a directory
# we can then glob all *.java to build rules in case the java wrapper becomes broken
#execute_process(COMMAND ${SWIG_EXECUTABLE}
#  -java
#  -I${GDCM_BINARY_DIR}/Source/Common
#  -I${GDCM_SOURCE_DIR}/Source/Common
#  -I${GDCM_SOURCE_DIR}/Source/DataStructureAndEncodingDefinition
#  -I${GDCM_SOURCE_DIR}/Source/MediaStorageAndFileFormat
#  -I${GDCM_SOURCE_DIR}/Source/DataDictionary
#  -c++
#  -o dummy.o
#  ${CMAKE_CURRENT_SOURCE_DIR}/gdcm.i
#  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#  RESULT_VARIABLE res
#)
#message("res:${res}")
#file(GLOB javafiles "${CMAKE_CURRENT_BINARY_DIR}/*.java")
#message("javafile:${javafiles}")

if(CMAKE_COMPILER_IS_GNUCXX)
  # http://www.swig.org/Doc2.0/Java.html#Java_typemaps_c_to_java_types
  set_source_files_properties(
    ${swig_generated_file_fullname}
    PROPERTIES COMPILE_FLAGS "-fno-strict-aliasing")
endif()

if(GDCM_JAVA_INCLUDE_SRC)
  set(GDCM_JAVA_SRCS "gdcm/*.java")
endif()

add_custom_command(
  OUTPUT ${GDCM_LIBRARY_DIR}/gdcm.jar
# 1. run the custom command only when the gdcmJAVA_wrap.cxx has been generated
# (which means all *.java should be there) Some compiler are picky about Java
# version. For instance covariant return type was only added to Java 5, so to
# please compiler such as ecj (Eclipse Java Compiler 0.894_R34x) explicitly
# state we want 1.5 version to compile gdcm::ImageToImageFilter::GetOutput()
# const as covariant return type
  COMMAND ${Java_JAVAC_EXECUTABLE} ARGS -source ${GDCM_JAVA_SOURCE_VERSION} -target ${GDCM_JAVA_TARGET_VERSION} "gdcm/*.java" #${javafiles}
# 2. now that the *.class have been generated construct the jar file. We can
# only rely on the gdcm.java / gdcm.class to build dependencie, I am pretty
# sure it will break parallel builds... oh well
  COMMAND ${Java_JAR_EXECUTABLE} ARGS cf ${GDCM_LIBRARY_DIR}/gdcm.jar gdcm/*.class ${GDCM_JAVA_SRCS}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/java
  DEPENDS "${swig_generated_file_fullname}"
  COMMENT "javac *.java; jar cf -> gdcm.jar"
)

if(GDCM_BUILD_FULL_JAVA_JAR)
  if(NOT GDCM_AUTOLOAD_GDCMJNI)
    message(FATAL_ERROR "Dont know how to handle full jar without autoload option")
  endif()
  if(BUILD_SHARED_LIBS)
    message(FATAL_ERROR "Dont know how to package gdcmjni with shared gdcm core libs.")
  endif()
  set(GDCM_JAVA_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
  set(GDCM_JAVA_JAR_FULL gdcm-${GDCM_VERSION}-${GDCM_JAVA_SYSTEM_NAME}.jar)
  add_custom_command(
    OUTPUT ${GDCM_LIBRARY_DIR}/${GDCM_JAVA_JAR_FULL}
    #COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/java/lib
    COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different $<TARGET_FILE:${SWIG_MODULE_gdcmjni_REAL_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/java/
    COMMAND ${Java_JAR_EXECUTABLE} ARGS cf ${GDCM_LIBRARY_DIR}/${GDCM_JAVA_JAR_FULL} gdcm/*.class ${GDCM_JAVA_SRCS} *gdcmjni*
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/java
    DEPENDS ${GDCM_LIBRARY_DIR}/gdcm.jar
    COMMENT "javac *.java; jar cf -> ${GDCM_JAVA_JAR_FULL}"
  )
endif()

# 3. ok now add the target
add_custom_target(GDCMJavaJar ALL
  DEPENDS ${GDCM_LIBRARY_DIR}/gdcm.jar
  ${SWIG_MODULE_gdcmjni_REAL_NAME}
  ${GDCM_LIBRARY_DIR}/${GDCM_JAVA_JAR_FULL}
  COMMENT "building gdcm.jar"
)

if(NOT GDCM_INSTALL_NO_LIBRARIES)
  install_swig_module(gdcmjni Java)
  set(GDCM_LIBRARY_DIR2 ${LIBRARY_OUTPUT_PATH}/\${BUILD_TYPE})
  # because gdcm.jar is constructed with custom commands, it need the install(FILES signature:
  install(FILES ${GDCM_LIBRARY_DIR2}/gdcm.jar
    DESTINATION ${GDCM_INSTALL_JARMODULE_DIR} COMPONENT JavaModule
  )
  if(GDCM_BUILD_FULL_JAVA_JAR)
    install(FILES ${GDCM_LIBRARY_DIR2}/${GDCM_JAVA_JAR_FULL}
      DESTINATION ${GDCM_INSTALL_JARMODULE_DIR} COMPONENT JavaModule
    )
  endif()
endif()
