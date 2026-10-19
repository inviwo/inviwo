/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025-2026 Inviwo Foundation
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

#include <inviwo/core/io/inviwofileformattypes.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

std::string_view enumToStr(ByteOrder byteOrder) {
    switch (byteOrder) {
        case ByteOrder::LittleEndian: return "LittleEndian";
        case ByteOrder::BigEndian:    return "BigEndian";
    }
    throw Exception{SourceContext{}, "Found invalid ByteOrder enum value '{}'",
                    static_cast<int>(byteOrder)};
}

std::string_view enumToStr(Compression compression) {
    switch (compression) {
        case Compression::Disabled: return "Disabled";
        case Compression::Enabled:  return "Enabled";
    }
    throw Exception{SourceContext{}, "Found invalid Compression enum value '{}'",
                    static_cast<int>(compression)};
}

std::optional<ByteOrder> strToByteOrder(std::string_view str) {
    for (auto e : {ByteOrder::LittleEndian, ByteOrder::BigEndian}) {
        if (iCaseCmp(str, enumToStr(e))) {
            return e;
        }
    }
    return std::nullopt;
}
std::optional<Compression> strToCompression(std::string_view str) {
    for (auto e : {Compression::Disabled, Compression::Enabled}) {
        if (iCaseCmp(str, enumToStr(e))) {
            return e;
        }
    }
    return std::nullopt;
}

}  // namespace inviwo
