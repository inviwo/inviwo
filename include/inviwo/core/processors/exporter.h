/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2023 Inviwo Foundation
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
#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/io/datawriter.h>

#include <vector>
#include <string_view>
#include <optional>
#include <string>
#include <filesystem>

namespace inviwo {

class ProcessorNetwork;

/**
 * \brief A base class for a Processor that might export a file. For example a CanvasProcessor
 */
class IVW_CORE_API Exporter {
public:
    virtual ~Exporter() = default;

    /**
     * Export some content to `path/name.ext` where ext is the first ext on candidateExtensions that
     * is supported.
     * @returns a string to the path of the exported file, or std::nullopt if no matching
     * extensions were found
     */
    virtual std::optional<std::filesystem::path> exportFile(
        const std::filesystem::path& path, std::string_view name,
        const std::vector<FileExtension>& candidateExtensions, Overwrite overwrite) const = 0;
};

namespace util {

/**
 * Exports the data from all export processors in \p network into the directory \p dir using a \p
 * nameTemplate and candidate extensions.
 * @return names of exported files
 */
IVW_CORE_API std::vector<std::string> exportAllFiles(
    ProcessorNetwork& network, const std::filesystem::path& dir, std::string_view nameTemplate,
    const std::vector<FileExtension>& candidateExtensions, Overwrite overwrite);

}  // namespace util

}  // namespace inviwo
