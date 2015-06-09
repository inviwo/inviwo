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

#ifndef IVW_FORMATCONVERSION_H
#define IVW_FORMATCONVERSION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <string>
#include <sstream>

namespace inviwo {

#define BYTE_SWAP 1024

static const glm::u64 BYTE_SIZE = 1;
static const glm::u64 KILO_BYTE_SIZE = BYTE_SWAP* BYTE_SIZE;
static const glm::u64 MEGA_BYTE_SIZE = BYTE_SWAP* KILO_BYTE_SIZE;
static const glm::u64 GIGA_BYTE_SIZE = BYTE_SWAP* MEGA_BYTE_SIZE;
static const glm::u64 TERA_BYTE_SIZE = BYTE_SWAP* GIGA_BYTE_SIZE;
static const float BYTE_DIV = 1.f/BYTE_SWAP;

#define BYTES_TO_KILOBYTES(bytes) (bytes/BYTE_SWAP)
#define BYTES_TO_MEGABYTES(bytes) (bytes/(BYTE_SWAP*BYTE_SWAP))
#define KILOBYTES_TO_BYTES(bytes) (bytes*BYTE_SWAP)
#define MEGABYTES_TO_BYTES(bytes) (bytes*BYTE_SWAP*BYTE_SWAP)

#include <warn/push>
#include <warn/ignore/unused-function>
static std::string formatBytesToString(glm::u64 bytes) {
    std::ostringstream stream;
    stream.precision(2);
    stream.setf(std::ios::fixed, std::ios::floatfield);

    if (bytes > TERA_BYTE_SIZE)
        stream << static_cast<float>(bytes/GIGA_BYTE_SIZE)*BYTE_DIV << " TB";
    else if (bytes > GIGA_BYTE_SIZE)
        stream << static_cast<float>(bytes/MEGA_BYTE_SIZE)*BYTE_DIV << " GB";
    else if (bytes > MEGA_BYTE_SIZE)
        stream << static_cast<float>(bytes/KILO_BYTE_SIZE)*BYTE_DIV << " MB";
    else if (bytes > KILO_BYTE_SIZE)
        stream << static_cast<float>(bytes)*BYTE_DIV << " KB";
    else
        stream << static_cast<float>(bytes) << " B";

    return stream.str();
};
#include <warn/pop>

} // namespace

#endif // IVW_FORMATCONVERSION_H
