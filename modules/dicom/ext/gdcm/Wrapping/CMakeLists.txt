# python
if(GDCM_WRAP_PYTHON)
  add_subdirectory(Python)
endif()
# java
if(GDCM_WRAP_JAVA)
  add_subdirectory(Java)
endif()
# php
if(GDCM_WRAP_PHP)
  add_subdirectory(PHP)
endif()
# perl
if(GDCM_WRAP_PERL)
  add_subdirectory(Perl)
endif()
# C#
if(GDCM_WRAP_CSHARP)
#  add_subdirectory(Csharp)
endif()
