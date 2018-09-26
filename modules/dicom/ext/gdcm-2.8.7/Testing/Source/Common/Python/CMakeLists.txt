# Define the tests for gdcm-python
# gdcm-python
set(GDCM_PYTHON_TEST_SRCS
  TestTesting
  )
if(GDCM_DATA_ROOT)
  set(GDCM_PYTHON_TEST_SRCS
    ${GDCM_PYTHON_TEST_SRCS}
    TestDirectory
  )
endif()

# Loop over files and create executables
foreach(name ${GDCM_PYTHON_TEST_SRCS})
  ADD_PYTHON_TEST(${name}Python ${name}.py ${GDCM_DATA_ROOT}/test.acr)
endforeach()
