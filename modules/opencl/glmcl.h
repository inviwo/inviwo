/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef __GLMCL_H__
#define __GLMCL_H__

#include <modules/opencl/cl.hpp>
#include <glm/glm.hpp>
#include <modules/opencl/openclmoduledefine.h>
#include <inviwo/core/util/logcentral.h>
#include <iostream>

namespace cl {


// Kernel argument specializations for vec3 types:
// int, uint, float

// int types
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const glm::i8vec3& value);
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const glm::i16vec3& value);
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const glm::i32vec3& value);
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const glm::i64vec3& value);

// uint types
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const glm::u8vec3& value);
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const glm::u16vec3& value);
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const glm::u32vec3& value);
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const glm::u64vec3& value);


// float types
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const glm::vec3& value);

} // namespace cl
#endif
