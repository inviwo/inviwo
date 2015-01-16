/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/glmcl.h>

namespace cl {

// Kernel argument specializations for vec3 types:
// int, uint, float

// int types
template <>
cl_int Kernel::setArg(cl_uint index, const glm::i8vec3& value)
{
    return setArg(index, glm::i8vec4(value, 0));
}
template <>
cl_int Kernel::setArg(cl_uint index, const glm::i16vec3& value)
{
    return setArg(index, glm::i16vec4(value, 0));
}
template <>
cl_int Kernel::setArg(cl_uint index, const glm::i32vec3& value)
{
    return setArg(index, glm::i32vec4(value, 0));
}
template <>
cl_int Kernel::setArg(cl_uint index, const glm::i64vec3& value)
{
    return setArg(index, glm::i64vec4(value, 0));
}

// uint types
template <>
cl_int Kernel::setArg(cl_uint index, const glm::u8vec3& value)
{
    return setArg(index, glm::u8vec4(value, 0));
}
template <>
cl_int Kernel::setArg(cl_uint index, const glm::u16vec3& value)
{
    return setArg(index, glm::u16vec4(value, 0));
}
template <>
cl_int Kernel::setArg(cl_uint index, const glm::u32vec3& value)
{
    return setArg(index, glm::u32vec4(value, 0));
}
template <>
cl_int Kernel::setArg(cl_uint index, const glm::u64vec3& value)
{
    return setArg(index, glm::u64vec4(value, 0));
}


// float types
template <>
cl_int Kernel::setArg(cl_uint index, const glm::vec3& value)
{
    return setArg(index, glm::vec4(value, 0));
}





} // namespace cl
