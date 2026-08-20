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

// macOS backend. iCloud Drive items are queried and controlled through Foundation's NSURL resource
// values and NSFileManager. Files provided by any File Provider extension (including third party
// providers such as OneDrive on modern macOS) are additionally detected via the SF_DATALESS stat
// flag which marks a file whose contents are not currently materialized on disk.

#include "cloudfilesdetail.hpp"

#include <inviwo/cloudfiles/cloudfiles.hpp>

#import <Foundation/Foundation.h>

#include <sys/stat.h>

#include <filesystem>
#include <optional>
#include <string>
#include <system_error>

#include <fmt/format.h>
#include <fmt/std.h>

// SF_DATALESS may be missing from older SDK headers.
#ifndef SF_DATALESS
#define SF_DATALESS 0x40000000
#endif

namespace inviwo::cloudfiles::detail {

namespace {

NSURL* toURL(const std::filesystem::path& path) {
    NSString* str = [NSString stringWithUTF8String:path.c_str()];
    return [NSURL fileURLWithPath:str];
}

bool isDataless(const std::filesystem::path& path) {
    struct stat sb {};
    if (::lstat(path.c_str(), &sb) != 0) return false;
    return (sb.st_flags & SF_DATALESS) != 0;
}

bool isUbiquitous(NSURL* url) {
    NSNumber* value = nil;
    [url getResourceValue:&value forKey:NSURLIsUbiquitousItemKey error:nil];
    return value != nil && [value boolValue];
}

std::string describeError(NSError* err) {
    if (err == nil) return "unknown error";
    const long code = static_cast<long>([err code]);
    std::string desc = fmt::format("{} (domain {}, code {})",
                                   [[err localizedDescription] UTF8String],
                                   [[err domain] UTF8String], code);
    // Cocoa 257/513 from the iCloud daemon means the process was denied by macOS privacy (TCC).
    if ([[err domain] isEqualToString:NSCocoaErrorDomain] &&
        (code == NSFileReadNoPermissionError || code == NSFileWriteNoPermissionError)) {
        desc +=
            ". macOS denied the operation: grant the host application Full Disk Access "
            "(System Settings > Privacy & Security), or add the iCloud entitlements when sandboxed.";
    }
    return desc;
}

}  // namespace

bool isSupported() noexcept { return true; }

Status status(const std::filesystem::path& path) {
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        throw CloudFilesError(fmt::format("File does not exist: {}", path));
    }

    Status result;
    if (const auto size = std::filesystem::file_size(path, ec); !ec) {
        result.size = size;
    }

    const bool dataless = isDataless(path);

    @autoreleasepool {
        NSURL* url = toURL(path);
        const bool ubiquitous = isUbiquitous(url);
        result.isPlaceholder = ubiquitous || dataless;

        if (!ubiquitous) {
            result.availability =
                dataless ? Availability::NotAvailable : Availability::NotACloudFile;
            return result;
        }

        NSNumber* downloading = nil;
        [url getResourceValue:&downloading
                       forKey:NSURLUbiquitousItemIsDownloadingKey
                        error:nil];
        if (downloading != nil && [downloading boolValue]) {
            result.availability = Availability::Downloading;
            return result;
        }

        NSString* downloadStatus = nil;
        [url getResourceValue:&downloadStatus
                       forKey:NSURLUbiquitousItemDownloadingStatusKey
                        error:nil];
        if (downloadStatus == nil) {
            result.availability = dataless ? Availability::NotAvailable : Availability::Available;
        } else if ([downloadStatus isEqualToString:NSURLUbiquitousItemDownloadingStatusNotDownloaded]) {
            result.availability = Availability::NotAvailable;
        } else {
            // NSURLUbiquitousItemDownloadingStatusDownloaded or ...Current
            result.availability = Availability::Available;
        }
    }
    return result;
}

void download(const std::filesystem::path& path) {
    std::optional<std::string> error;

    @autoreleasepool {
        NSURL* url = toURL(path);
        // A coordinated read blocks until the File Provider (iCloud, OneDrive, Dropbox, ...) has
        // materialized the file locally. Unlike startDownloadingUbiquitousItemAtURL: (iCloud only)
        // this works for any provider and needs only read access, not an iCloud entitlement.
        NSFileCoordinator* coordinator = [[NSFileCoordinator alloc] initWithFilePresenter:nil];
        NSError* coordinationError = nil;
        __block NSError* accessorError = nil;
        [coordinator coordinateReadingItemAtURL:url
                                        options:0
                                          error:&coordinationError
                                     byAccessor:^(NSURL* readingURL) {
                                         NSError* readError = nil;
                                         NSFileHandle* handle =
                                             [NSFileHandle fileHandleForReadingFromURL:readingURL
                                                                                 error:&readError];
                                         if (handle == nil) {
                                             accessorError = readError;
                                             return;
                                         }
                                         [handle readDataOfLength:1];  // fault the data in
                                         [handle closeFile];
                                     }];
        [coordinator release];

        NSError* failure = coordinationError != nil ? coordinationError : accessorError;
        if (failure != nil) {
            error = fmt::format("download failed for '{}': {}", path.string(),
                                describeError(failure));
        }
    }

    if (error) throw CloudFilesError(*error);
}

void offload(const std::filesystem::path& path) {
    // Third-party File Providers under ~/Library/CloudStorage (OneDrive, Dropbox, Google Drive, ...)
    // have no public eviction API on macOS; only iCloud items can be evicted programmatically.
    if (path.string().find("/Library/CloudStorage/") != std::string::npos) {
        throw CloudFilesError(fmt::format(
            "Offloading is not supported for third-party cloud providers such as OneDrive on macOS; "
            "use the provider's Finder extension ('Free up space') instead: {}",
            path.string()));
    }

    std::optional<std::string> error;

    @autoreleasepool {
        NSURL* url = toURL(path);
        if (!isUbiquitous(url)) {
            error = fmt::format("Offloading is only supported for iCloud items: {}", path.string());
        } else {
            NSError* err = nil;
            if (![[NSFileManager defaultManager] evictUbiquitousItemAtURL:url error:&err]) {
                error = fmt::format("evictUbiquitousItem failed for '{}': {}", path.string(),
                                    describeError(err));
            }
        }
    }

    if (error) throw CloudFilesError(*error);
}

}  // namespace inviwo::cloudfiles::detail
