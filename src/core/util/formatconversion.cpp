/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <inviwo/core/util/formatconversion.h>
#include <sstream>

namespace inviwo {

glm::u64 util::bytes_to_kilobytes(glm::u64 bytes) { return bytes / byte_swap; }

glm::u64 util::bytes_to_megabytes(glm::u64 bytes) { return bytes / (byte_swap * byte_swap); }

glm::u64 util::kilobytes_to_bytes(glm::u64 bytes) { return bytes * byte_swap; }

glm::u64 util::megabytes_to_bytes(glm::u64 bytes) { return bytes * byte_swap * byte_swap; }

std::string util::formatBytesToString(glm::u64 bytes) {
    std::ostringstream stream;
    stream.precision(2);
    stream.setf(std::ios::fixed, std::ios::floatfield);

    if (bytes > tera_byte_size)
        stream << static_cast<float>(bytes / giga_byte_size) * byte_div << " TB";
    else if (bytes > giga_byte_size)
        stream << static_cast<float>(bytes / mega_byte_size) * byte_div << " GB";
    else if (bytes > mega_byte_size)
        stream << static_cast<float>(bytes / kilo_byte_size) * byte_div << " MB";
    else if (bytes > kilo_byte_size)
        stream << static_cast<float>(bytes) * byte_div << " KB";
    else
        stream << static_cast<float>(bytes) << " B";

    return stream.str();
}

}  // namespace
