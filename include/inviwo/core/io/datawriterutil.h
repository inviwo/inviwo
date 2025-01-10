/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2025 Inviwo Foundation
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
#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/io/datawriterexception.h>

#include <string_view>
#include <vector>
#include <optional>
#include <fmt/std.h>

namespace inviwo {

namespace util {

/**
 * Save @p data to @p filePath using the writer given by @p extension
 * @param data Object to save to file
 * @param filePath Complete path, name, and extension of the written file
 * @param extension The extension used to identifier the DataWriter in the factory
 * @param overwrite Whether to overwrite any existing file or not
 * @throws DataWriterException if no write could be found, or if overwrite was violated.
 */
template <typename T>
void saveData(const T& data, const std::filesystem::path& filePath, const FileExtension& extension,
              Overwrite overwrite) {
    auto factory = util::getDataWriterFactory();
    if (auto writer = factory->getWriterForTypeAndExtension<T>(extension, filePath)) {
        writer->setOverwrite(overwrite);
        writer->writeData(&data, filePath);
    } else {
        throw DataWriterException(
            fmt::format("Could not find a writer for {} of the specified extension {}", filePath,
                        extension.toString()),
            IVW_CONTEXT_CUSTOM("datawriterutil"));
    }
}

/**
 * Save @p data to the filePath given by concatenating @p path, "/", @p name, ".", and extension
 * where extensions is the first extension in @p extensions that we find a matching writer for.
 * @param data Object to save to file
 * @param path Directory where the file will be written
 * @param name Name of the file excluding extension
 * @param extensions A list of extensions to use for finding a matching DataWriter
 * @param overwrite Whether to overwrite any existing file or not
 * @returns The full path to the file written or std::nullopt if no writer was found
 * @throws DataWriterException if overwrite was violated.
 */
template <typename T>
std::optional<std::filesystem::path> saveData(const T& data, const std::filesystem::path& path,
                                              std::string_view name,
                                              const std::vector<FileExtension>& extensions,
                                              Overwrite overwrite) {
    auto factory = util::getDataWriterFactory();

    for (const auto& extension : extensions) {
        if (auto writer = factory->getWriterForTypeAndExtension<T>(extension.extension_)) {
            writer->setOverwrite(overwrite);
            const auto file = path / fmt::format("{}.{}", name, extension.extension_);
            writer->writeData(&data, file);
            return file;
        }
    }
    return std::nullopt;
}

}  // namespace util

}  // namespace inviwo
