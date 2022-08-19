/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/core/processors/exporter.h>

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/stringconversion.h>

#include <fmt/format.h>

namespace inviwo {

std::vector<std::string> util::exportAllFiles(ProcessorNetwork& network, std::string_view path,
                                              std::string_view nameTemplate,
                                              const std::vector<FileExtension>& candidateExtensions,
                                              Overwrite overwrite) {

    std::vector<std::string> exportedFiles;
    network.forEachProcessor([&](Processor* p) {
        if (auto exporter = dynamic_cast<const Exporter*>(p)) {
            if (!p->isValid()) {
                throw Exception(IVW_CONTEXT_CUSTOM("util::exportAllFiles"),
                                "Processor {} is not valid, no file exported", p->getIdentifier());
            }
            if (!p->isReady()) {
                throw Exception(IVW_CONTEXT_CUSTOM("util::exportAllFiles"),
                                "Processor {} is not ready, no file exported", p->getIdentifier());
            }

            StrBuffer name;
            if (nameTemplate.empty()) {
                name.append("{}", p->getIdentifier());
            } else if (nameTemplate.find("UPN") != std::string::npos) {
                auto [before, after] = util::splitByFirst(nameTemplate, "UPN");
                name.append("{}{}{}", before, p->getIdentifier(), after);
            } else {
                name.append("{}{:03}", nameTemplate, exportedFiles.size());
            }

            if (auto file = exporter->exportFile(path, name, candidateExtensions, overwrite)) {
                exportedFiles.push_back(*file);
            } else {
                throw Exception(IVW_CONTEXT_CUSTOM("util::exportAllFiles"),
                                "No matching extension found for Exporter/Processor {}",
                                p->getIdentifier());
            }
        }
    });

    return exportedFiles;
}

}  // namespace inviwo
