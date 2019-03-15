/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_FORMATCONVERSION_H
#define IVW_FORMATCONVERSION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <string>

namespace inviwo {

namespace util {

static const glm::u64 byte_swap = 1024;
static const glm::u64 byte_size = 1;
static const glm::u64 kilo_byte_size = byte_swap * byte_size;
static const glm::u64 mega_byte_size = byte_swap * kilo_byte_size;
static const glm::u64 giga_byte_size = byte_swap * mega_byte_size;
static const glm::u64 tera_byte_size = byte_swap * giga_byte_size;
static const double byte_div = 1.0 / byte_swap;

IVW_CORE_API glm::u64 bytes_to_kilobytes(glm::u64 bytes);
IVW_CORE_API glm::u64 bytes_to_megabytes(glm::u64 bytes);
IVW_CORE_API glm::u64 kilobytes_to_bytes(glm::u64 bytes);
IVW_CORE_API glm::u64 megabytes_to_bytes(glm::u64 bytes);

IVW_CORE_API std::string formatBytesToString(glm::u64 bytes);

}  // namespace util

}  // namespace inviwo

#endif  // IVW_FORMATCONVERSION_H
