# Copyright (c) 2016 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

# Determine the platform used by CEF.
function(determineCEFPlatform CEF_PLATFORM)
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  set(CEF_PLATFORM "macosx64"  PARENT_SCOPE)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(CEF_PLATFORM "linux64" PARENT_SCOPE)
  else()
    set(CEF_PLATFORM "linux32" PARENT_SCOPE)
  endif()
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(CEF_PLATFORM "windows64" PARENT_SCOPE)
  else()
    set(CEF_PLATFORM "windows32" PARENT_SCOPE)
  endif()
endif()
endfunction()