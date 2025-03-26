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

#include <inviwo/core/io/bytewriterutil.h>
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

void writeUncompressedBytes(const std::filesystem::path& path, const void* source, size_t bytes) {
    FILE* file = filesystem::fopen(path, "wb");
    if (!file) {
        throw DataReaderException(SourceContext{}, "Could not open file: {:?g}", path);
    }
    const util::OnScopeExit closeFile{[file]() { std::fclose(file); }};

    if (std::fwrite(source, bytes, 1, file) != 1) {
        throw DataReaderException(SourceContext{}, "Could not write to file: {:?g}", path);
    }
}

void writeCompressedBytes(const std::filesystem::path& path, const void* source, size_t bytes) {
    auto fout = bxz::ofstream{path.generic_string(), bxz::z};

    if (fout.good()) {
        try {
            fout.write(static_cast<const char*>(source), static_cast<std::streamsize>(bytes));
        } catch (const std::ios_base::failure& e) {
            throw DataReaderException(SourceContext{}, "Could not write to file: {:?g}", path);
        }
    } else {
        throw DataReaderException(SourceContext{}, "Could not open file: {:?g}", path);
    }
}

}  // namespace

void util::writeBytes(const std::filesystem::path& path, const void* source, size_t bytes,
                      Compression compression) {
    if (compression == Compression::Enabled && util::isCompressionSupported()) {
        writeCompressedBytes(path, source, bytes);
    } else {
        writeUncompressedBytes(path, source, bytes);
    }
}

}  // namespace inviwo
