/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <inviwo/core/io/bytereaderutil.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/curlutils.h>
#include <inviwo/core/io/inviwofileformattypes.h>

#include <bxzstr/bxzstr.hpp>

#include <fmt/format.h>
#include <fmt/std.h>

#include <memory>

namespace inviwo {

namespace {

void convertToLittleEndian(void* dest, size_t bytes, size_t elementSize) {
    auto temp = std::make_unique<char[]>(elementSize);

    for (std::size_t i = 0; i < bytes; i += elementSize) {
        for (std::size_t j = 0; j < elementSize; j++) temp[j] = static_cast<char*>(dest)[i + j];

        for (std::size_t j = 0; j < elementSize; j++)
            static_cast<char*>(dest)[i + j] = temp[elementSize - j - 1];
    }
}

}  // namespace

void util::readBytesIntoBuffer(const std::filesystem::path& path, size_t offset, size_t bytes,
                               ByteOrder byteOrder, size_t elementSize, void* dest) {
    const auto filePath = net::downloadAndCacheIfUrl(path);

    FILE* file = filesystem::fopen(filePath, "rb");
    if (!file) {
        throw DataReaderException(SourceContext{}, "Could not open file: {:?g}", path);
    }
    const util::OnScopeExit closeFile{[file]() { std::fclose(file); }};

    std::fseek(file, static_cast<long>(offset), SEEK_SET);
    if (std::fread(static_cast<char*>(dest), bytes, 1, file) != 1) {
        throw DataReaderException(SourceContext{}, "Could not read from file: {:?g}", path);
    }
    if (byteOrder == ByteOrder::BigEndian && elementSize > 1) {
        convertToLittleEndian(dest, bytes, elementSize);
    }
}

void util::readCompressedBytesIntoBuffer(const std::filesystem::path& path, size_t offset,
                                         size_t bytes, ByteOrder byteOrder, size_t elementSize,
                                         void* dest) {
    const auto filePath = net::downloadAndCacheIfUrl(path);

    auto fin = bxz::ifstream{filePath.generic_string(), std::ios::in | std::ios::binary};

    if (fin.good()) {
        fin.seekg(static_cast<std::streamoff>(offset));
        fin.read(static_cast<char*>(dest), static_cast<std::streamsize>(bytes));

        if (byteOrder == ByteOrder::BigEndian && elementSize > 1) {
            convertToLittleEndian(dest, bytes, elementSize);
        }
    } else {
        throw DataReaderException(SourceContext{}, "Could not read from file: {:?g}", path);
    }
}

}  // namespace inviwo
