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

#pragma once

#include <inviwo/cloudfiles/cloudfilesdefine.hpp>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace inviwo::cloudfiles {

/**
 * Local availability of a file that may be managed by a cloud storage provider such as
 * OneDrive (Windows) or iCloud Drive (macOS).
 */
enum class Availability {
    NotACloudFile,  //!< A regular local file, not managed by a cloud provider.
    Available,      //!< Managed by a provider and fully materialized (hydrated) on disk.
    NotAvailable,   //!< A placeholder/dehydrated file, its data is not stored locally.
    Downloading,    //!< The provider is currently downloading the file.
    Unknown,        //!< The state could not be determined (e.g. unsupported platform).
};

/// Human readable name of an Availability value.
INVIWO_CLOUDFILES_API std::string_view toString(Availability availability);

/**
 * The result of querying a file with @ref status.
 */
struct INVIWO_CLOUDFILES_API Status {
    Availability availability = Availability::Unknown;
    /// True if the file is a cloud provider placeholder (on-demand file). A hydrated file may
    /// look like a regular local file on some platforms, in which case this is false.
    bool isPlaceholder = false;
    /// Logical size of the file in bytes, if it could be determined.
    std::optional<std::uintmax_t> size;
};

/**
 * Thrown by the cloudfiles functions when an operation fails.
 */
class INVIWO_CLOUDFILES_API CloudFilesError : public std::runtime_error {
public:
    explicit CloudFilesError(const std::string& message);
    explicit CloudFilesError(const char* message);
};

/// Whether the current platform has a real cloud provider backend.
INVIWO_CLOUDFILES_API bool isSupported() noexcept;

/// Query the availability of @p path. Throws @ref CloudFilesError if the file does not exist.
INVIWO_CLOUDFILES_API Status status(const std::filesystem::path& path);

/// Convenience for `status(path).availability` being Available or NotACloudFile.
INVIWO_CLOUDFILES_API bool isLocallyAvailable(const std::filesystem::path& path);

/// Convenience for `status(path).isPlaceholder`, i.e. whether the file is managed on-demand.
INVIWO_CLOUDFILES_API bool isCloudFile(const std::filesystem::path& path);

/**
 * Trigger a download (hydration) of @p path so that its data becomes available locally. On some
 * platforms this only starts an asynchronous download; use @ref status to observe progress.
 * Throws @ref CloudFilesError on failure.
 */
INVIWO_CLOUDFILES_API void download(const std::filesystem::path& path);

/**
 * Offload (dehydrate/evict) @p path to free up local disk space, keeping the file available
 * on-demand from the cloud provider. Throws @ref CloudFilesError on failure or when the operation
 * is not supported for the given file.
 */
INVIWO_CLOUDFILES_API void offload(const std::filesystem::path& path);

}  // namespace inviwo::cloudfiles
