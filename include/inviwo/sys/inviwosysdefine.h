/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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

#pragma once

#ifdef INVIWO_ALL_DYN_LINK  // DYNAMIC
// If we are building DLL files we must declare dllexport/dllimport
#ifdef IVW_SYS_EXPORTS
#ifdef _WIN32
#define IVW_SYS_API __declspec(dllexport)
#define IVW_SYS_EXT
#define IVW_SYS_TMPL_EXP
#define IVW_SYS_TMPL_INST __declspec(dllexport)
#else  // UNIX (GCC)
#define IVW_SYS_API __attribute__((visibility("default")))
#define IVW_SYS_EXT
#define IVW_SYS_TMPL_EXP __attribute__((__visibility__("default")))
#define IVW_SYS_TMPL_INST
#endif
#else
#ifdef _WIN32
#define IVW_SYS_API __declspec(dllimport)
#define IVW_SYS_EXT extern
#define IVW_SYS_TMPL_EXP __declspec(dllimport)
#define IVW_SYS_TMPL_INST
#else
#define IVW_SYS_API
#define IVW_SYS_EXT extern
#define IVW_SYS_TMPL_EXP __attribute__((__visibility__("default")))
#define IVW_SYS_TMPL_INST
#endif
#endif
#else  // STATIC
#define IVW_SYS_API
#define IVW_SYS_EXT extern
#define IVW_SYS_TMPL_EXP
#define IVW_SYS_TMPL_INST
#endif
