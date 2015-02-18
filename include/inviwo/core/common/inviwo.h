/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 *********************************************************************************/

#ifndef IVW_INVIWO_H
#define IVW_INVIWO_H

#pragma warning(disable: 4290)

//#pragma warning(default: 4640) // non thread safe construction of object
#pragma warning(default: 4514) // never used function
#pragma warning(default: 4296) // expression is always false
#pragma warning(default: 4061) // enum
#pragma warning(default: 4062) // enum
#pragma warning(default: 4266) // warn if no overide of function in baseclass
#pragma warning(default: 4265) // warn if non-virtual destructor
#pragma warning(default: 4263) // warn for virtual functions that do not overide something in baseclass

#define NOMINMAX

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// Define a debug flag for inviwo called IVW_DEBUG
#ifdef __clang__ // Clang sets NDEBUG when not debugging
    #ifndef NDEBUG
        #ifndef IVW_DEBUG
            #define IVW_DEBUG
        #endif
    #endif
#endif

#ifdef _MSC_VER
    #ifdef _DEBUG /// MVS sets _DEBUG when debugging
        #ifndef IVW_DEBUG
            #define IVW_DEBUG
        #endif
    #endif
#endif

// Convenience macros for clang warning pragmas
#ifdef __clang__
#define STARTCLANGIGNORESTR(X) #X
#define STARTCLANGIGNORE(X) \
    _Pragma("clang diagnostic push") \
    _Pragma(STARTCLANGIGNORESTR(clang diagnostic ignored X))
#define ENDCLANGIGNORE \
    _Pragma("clang diagnostic pop")
#else
#define STARTCLANGIGNORE(X)
#define ENDCLANGIGNORE
#endif

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <stdio.h>

#include <inviwo/core/common/inviwocommondefines.h>

// include glm
#include <inviwo/core/util/glm.h>

#include <inviwo/core/util/formats.h>

// error handling
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/logcentral.h>
#define IVW_UNUSED_PARAM(param) (void)param

#include <inviwo/core/io/serialization/ivwserialization.h>

//#define IVW_DEPRECATION_WARNINGS

#if defined(IVW_DEPRECATION_WARNINGS)
#define ivwDeprecatedMethod(newFunction) \
    std::cout << __FUNCTION__ << " is deprecated. Use " << newFunction << " instead." << std::endl; \
    std::cout << "(" << __FILE__ << " - Line " << __LINE__ << ")." << std::endl;
#else
#define ivwDeprecatedMethod(newFunction)
#endif

#ifdef _MSC_VER
#ifdef IVW_ENABLE_MSVC_MEM_LEAK_TEST
#include <ext/vld/vld.h>
#endif
#endif

#endif // IVW_INVIWO_H
