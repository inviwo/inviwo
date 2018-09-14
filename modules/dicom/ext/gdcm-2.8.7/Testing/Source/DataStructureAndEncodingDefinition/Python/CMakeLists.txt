# Define the tests for gdcm-python
# gdcm-python
set(GDCM_PYTHON_TEST_SRCS
  TestTag
  )
if(GDCM_DATA_ROOT)
  set(GDCM_PYTHON_TEST_SRCS
    ${GDCM_PYTHON_TEST_SRCS}
    TestReader
  )
endif()

# Loop over files and create executables
foreach(name ${GDCM_PYTHON_TEST_SRCS})
  ADD_PYTHON_TEST(${name}Python ${name}.py)
endforeach()
