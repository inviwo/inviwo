/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <inviwo/core/io/datareader.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>
#include <fmt/format.h>

#include <inviwo/core/io/curlutils.h>

#include <fmt/std.h>

namespace inviwo {

const std::vector<FileExtension>& DataReader::getExtensions() const { return extensions_; }
void DataReader::addExtension(FileExtension ext) { extensions_.push_back(ext); }

void DataReader::checkExists(const std::filesystem::path& path) {
    if (!std::filesystem::is_regular_file(path)) {
        throw DataReaderException(SourceContext{}, "Could not find input file: {}", path);
    }
}

std::ifstream DataReader::open(const std::filesystem::path& path, std::ios_base::openmode mode) {
    checkExists(path);
    if (auto file = std::ifstream(path, mode)) {
        return file;
    } else {
        throw FileException(SourceContext{}, "Could not open file: {}", path);
    }
}

std::filesystem::path DataReader::downloadAndCacheIfUrl(const std::filesystem::path& url) {
    return net::downloadAndCacheIfUrl(url);
}

std::ifstream DataReader::openAndCacheIfUrl(const std::filesystem::path& path,
                                            std::ios_base::openmode mode) {
    const auto localPath = net::downloadAndCacheIfUrl(path);

    checkExists(localPath);
    if (auto file = std::ifstream(localPath, mode)) {
        return file;
    } else {
        throw FileException(SourceContext{}, "Could not open file: {}", path);
    }
}

}  // namespace inviwo
