/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/cloudfiles/cloudfiles.hpp>

#include "cloudfilesdetail.hpp"

namespace inviwo::cloudfiles {

CloudFilesError::CloudFilesError(const std::string& message) : std::runtime_error(message) {}
CloudFilesError::CloudFilesError(const char* message) : std::runtime_error(message) {}

std::string_view toString(Availability availability) {
    switch (availability) {
        case Availability::NotACloudFile:
            return "NotACloudFile";
        case Availability::Available:
            return "Available";
        case Availability::NotAvailable:
            return "NotAvailable";
        case Availability::Downloading:
            return "Downloading";
        case Availability::Unknown:
            return "Unknown";
    }
    return "Unknown";
}

bool isSupported() noexcept { return detail::isSupported(); }

Status status(const std::filesystem::path& path) { return detail::status(path); }

bool isLocallyAvailable(const std::filesystem::path& path) {
    const auto availability = detail::status(path).availability;
    return availability == Availability::Available || availability == Availability::NotACloudFile;
}

bool isCloudFile(const std::filesystem::path& path) { return detail::status(path).isPlaceholder; }

void download(const std::filesystem::path& path) { detail::download(path); }

void offload(const std::filesystem::path& path) { detail::offload(path); }

}  // namespace inviwo::cloudfiles
