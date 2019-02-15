# Define the tests for gdcm-java
set(GDCM_JAVA_TEST_SRCS
  TestReader
  TestTag
  TestReaderUpToTag
  )

# Do not change anything after here:
find_package(Java ${GDCM_DEFAULT_JAVA_VERSION} REQUIRED) # javac, jar
find_package(JNI REQUIRED)
include_directories(
  ${JNI_INCLUDE_PATH}
  )

set(classfilesdep)

foreach(example ${GDCM_JAVA_TEST_SRCS})
  add_custom_command(
    OUTPUT ${GDCM_EXECUTABLE_DIR}/${example}.class
    COMMAND ${Java_JAVAC_EXECUTABLE} ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${example}.java -d ${GDCM_EXECUTABLE_DIR} -source ${GDCM_JAVA_SOURCE_VERSION} -target ${GDCM_JAVA_TARGET_VERSION} -classpath ${GDCM_LIBRARY_DIR}/gdcm.jar
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${example}.java
    COMMENT "javac ${example}.java"
    )
  list(APPEND classfilesdep ${GDCM_EXECUTABLE_DIR}/${example}.class)
  set(GDCM_LIBRARY_DIR2 ${LIBRARY_OUTPUT_PATH}/\${BUILD_TYPE})
  set_source_files_properties(${GDCM_EXECUTABLE_DIR}/${example}.class PROPERTIES CLASSPATH "${GDCM_LIBRARY_DIR2}/gdcm.jar")
  ADD_JAVA_TEST(${example}Java ${GDCM_EXECUTABLE_DIR}/${example})
endforeach()

# 3. ok now add the target
add_custom_target(GDCMJavaDSEDTests ALL
  DEPENDS ${classfilesdep}
  COMMENT "building gdcm java dsed tests"
)
# make sure gdcm.jar is built
add_dependencies(GDCMJavaDSEDTests GDCMJavaJar)
