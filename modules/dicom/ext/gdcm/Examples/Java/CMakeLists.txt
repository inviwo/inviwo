find_package(Java ${GDCM_DEFAULT_JAVA_VERSION} REQUIRED) # javac, jar
find_package(JNI REQUIRED)
include_directories(
  #${JNI_INCLUDE_PATH}
  ${JAVA_INCLUDE_PATH}
  ${JAVA_INCLUDE_PATH2}
  ${JAVA_AWT_INCLUDE_PATH}
  )

set(classfilesdep)
set(examples
  HelloSimple
  ExtractImageRegion
  DecompressPixmap
  DecompressImage
  ScanDirectory
  ReadFiles
  FileAnonymize
  )
foreach(example ${examples})
  add_custom_command(
    OUTPUT ${GDCM_EXECUTABLE_DIR}/${example}.class
    COMMAND ${Java_JAVAC_EXECUTABLE} ARGS -source ${GDCM_JAVA_SOURCE_VERSION} -target ${GDCM_JAVA_TARGET_VERSION} ${CMAKE_CURRENT_SOURCE_DIR}/${example}.java -d ${GDCM_EXECUTABLE_DIR} -classpath ${GDCM_LIBRARY_DIR}/gdcm.jar
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${example}.java
    COMMENT "javac ${example}.java"
    )
  list(APPEND classfilesdep ${GDCM_EXECUTABLE_DIR}/${example}.class)
endforeach()

# 3. ok now add the target
add_custom_target(GDCMJavaExample ALL
  DEPENDS ${classfilesdep}
  COMMENT "building gdcm java example"
)
# make sure gdcm.jar is built
add_dependencies(GDCMJavaExample GDCMJavaJar)

if(BUILD_TESTING)
  if(GDCM_DATA_ROOT)
    set_source_files_properties(${GDCM_EXECUTABLE_DIR}/HelloSimple.class PROPERTIES CLASSPATH "${GDCM_LIBRARY_DIR}/gdcm.jar")
    ADD_JAVA_TEST(TestHelloSimpleJava ${GDCM_EXECUTABLE_DIR}/HelloSimple ${GDCM_DATA_ROOT}/012345.002.050.dcm)
  endif()
endif()
