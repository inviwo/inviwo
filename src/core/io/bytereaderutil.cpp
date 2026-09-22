/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2026 Inviwo Foundation
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

#include <algorithm>
#include <memory>

namespace inviwo {

namespace {

/// Number of bytes read between two cancellation checks.
constexpr size_t defaultChunkSize = 64 * 1024 * 1024;

size_t chunkSizeFor(size_t elementSize) {
    if (elementSize == 0 || elementSize > defaultChunkSize) return defaultChunkSize;
    return (defaultChunkSize / elementSize) * elementSize;
}

}  // namespace


void util::reverseByteOrder(void* dest, size_t bytes, size_t elementSize) {
    auto temp = std::make_unique<char[]>(elementSize);

    for (std::size_t i = 0; i < bytes; i += elementSize) {
        for (std::size_t j = 0; j < elementSize; j++) {
            temp[j] = static_cast<char*>(dest)[i + j];
        }

        for (std::size_t j = 0; j < elementSize; j++) {
            static_cast<char*>(dest)[i + j] = temp[elementSize - j - 1];
        }
    }
}


void util::readBytesIntoBuffer(const std::filesystem::path& path, size_t offset, size_t bytes,
                               ByteOrder byteOrder, size_t elementSize, void* dest,
                               std::stop_token stop) {
    const auto filePath = net::downloadAndCacheIfUrl(path);

    FILE* file = filesystem::fopen(filePath, "rb");
    if (!file) {
        throw DataReaderException(SourceContext{}, "Could not open file: {:?g}", path);
    }
    const util::OnScopeExit closeFile{[file]() { std::fclose(file); }};

    std::fseek(file, static_cast<long>(offset), SEEK_SET);

    const size_t chunk = chunkSizeFor(elementSize);
    auto* out = static_cast<char*>(dest);
    for (size_t read = 0; read < bytes; read += chunk) {
        if (stop.stop_requested()) return;
        const size_t count = std::min(chunk, bytes - read);
        if (std::fread(out + read, count, 1, file) != 1) {
            throw DataReaderException(SourceContext{}, "Could not read from file: {:?g}", path);
        }
    }
    if (byteOrder == ByteOrder::BigEndian && elementSize > 1) {
        util::reverseByteOrder(dest, bytes, elementSize);
    }
}

void util::readCompressedBytesIntoBuffer(const std::filesystem::path& path, size_t offset,
                                         size_t bytes, ByteOrder byteOrder, size_t elementSize,
                                         void* dest, std::stop_token stop) {
    const auto filePath = net::downloadAndCacheIfUrl(path);

    auto fin = bxz::ifstream{filePath.generic_string(), std::ios::in | std::ios::binary};
    if (!fin.good()) {
        throw DataReaderException(SourceContext{}, "Could not read from file: {:?g}", path);
    }

    fin.seekg(static_cast<std::streamoff>(offset));

    const size_t chunk = chunkSizeFor(elementSize);
    auto* out = static_cast<char*>(dest);
    for (size_t read = 0; read < bytes; read += chunk) {
        if (stop.stop_requested()) return;
        const size_t count = std::min(chunk, bytes - read);
        fin.read(out + read, static_cast<std::streamsize>(count));
    }

    if (byteOrder == ByteOrder::BigEndian && elementSize > 1) {
        util::reverseByteOrder(dest, bytes, elementSize);
    }
}

}  // namespace inviwo
