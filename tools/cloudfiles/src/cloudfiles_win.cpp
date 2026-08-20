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

// Windows backend using the Cloud Filter API (cfapi) together with the file placeholder
// attributes that OneDrive "Files On-Demand" sets. See
// https://learn.microsoft.com/windows/win32/cfapi/cloud-files-api-portal

#include "cloudfilesdetail.hpp"

#include <inviwo/cloudfiles/cloudfiles.hpp>

#include <windows.h>
#include <winternl.h>
#include <cfapi.h>

#include <filesystem>

#include <fmt/format.h>
#include <fmt/std.h>

namespace inviwo::cloudfiles::detail {

namespace {

[[noreturn]] void throwLastError(std::string_view what, const std::filesystem::path& path) {
    const DWORD err = ::GetLastError();
    throw std::runtime_error(
        fmt::format("{} failed for '{}' (error {})", what, path, static_cast<unsigned long>(err)));
}

[[noreturn]] void throwHResult(std::string_view what, const std::filesystem::path& path,
                               HRESULT hr) {
    throw std::runtime_error(fmt::format("{} failed for '{}' (hr 0x{:08x})", what, path,
                                         static_cast<unsigned long>(hr)));
}

DWORD getAttributes(const std::filesystem::path& path) {
    const DWORD attrs = ::GetFileAttributesW(path.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        throwLastError("GetFileAttributes", path);
    }
    return attrs;
}

// RAII wrapper for a Win32 file handle.
class FileHandle {
public:
    FileHandle(const std::filesystem::path& path, DWORD access) {
        handle_ = ::CreateFileW(path.c_str(), access,
                                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                                OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
        if (handle_ == INVALID_HANDLE_VALUE) {
            throwLastError("CreateFile", path);
        }
    }
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    ~FileHandle() {
        if (handle_ != INVALID_HANDLE_VALUE) ::CloseHandle(handle_);
    }
    HANDLE get() const { return handle_; }

private:
    HANDLE handle_ = INVALID_HANDLE_VALUE;
};

}  // namespace

bool isSupported() noexcept { return true; }

Status status(const std::filesystem::path& path) {
    const DWORD attrs = getAttributes(path);

    const bool recallOnData = (attrs & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS) != 0;
    const bool recallOnOpen = (attrs & FILE_ATTRIBUTE_RECALL_ON_OPEN) != 0;
    const bool offline = (attrs & FILE_ATTRIBUTE_OFFLINE) != 0;

    Status result;
    result.isPlaceholder = recallOnData || recallOnOpen || offline;

    if (recallOnData || recallOnOpen || offline) {
        // A dehydrated placeholder: opening/reading it would trigger a recall from the cloud.
        result.availability = Availability::NotAvailable;
    } else {
        // Either a fully hydrated placeholder or a regular local file; both have their data on
        // disk.
        result.availability = Availability::Available;
    }

    WIN32_FILE_ATTRIBUTE_DATA data{};
    if (::GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &data)) {
        ULARGE_INTEGER size;
        size.HighPart = data.nFileSizeHigh;
        size.LowPart = data.nFileSizeLow;
        result.size = static_cast<std::uintmax_t>(size.QuadPart);
    }

    return result;
}

void download(const std::filesystem::path& path) {
    const FileHandle handle{path, GENERIC_READ};

    LARGE_INTEGER offset;
    offset.QuadPart = 0;
    LARGE_INTEGER length;
    length.QuadPart = -1;  // hydrate the entire file

    const HRESULT hr =
        ::CfHydratePlaceholder(handle.get(), offset, length, CF_HYDRATE_FLAG_NONE, nullptr);
    if (FAILED(hr)) {
        // Not a cloud placeholder means the data is already local; treat as success.
        if (hr == HRESULT_FROM_WIN32(ERROR_NOT_A_CLOUD_FILE) ||
            hr == HRESULT_FROM_WIN32(ERROR_CLOUD_FILE_NOT_UNDER_SYNC_ROOT)) {
            return;
        }
        throwHResult("CfHydratePlaceholder", path, hr);
    }
}

void offload(const std::filesystem::path& path) {
    const FileHandle handle{path, GENERIC_READ | GENERIC_WRITE};

    LARGE_INTEGER offset;
    offset.QuadPart = 0;
    LARGE_INTEGER length;
    length.QuadPart = -1;  // dehydrate the entire file

    const HRESULT hr =
        ::CfDehydratePlaceholder(handle.get(), offset, length, CF_DEHYDRATE_FLAG_NONE, nullptr);
    if (FAILED(hr)) {
        throwHResult("CfDehydratePlaceholder", path, hr);
    }
}

}  // namespace inviwo::cloudfiles::detail
