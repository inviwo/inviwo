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

#include <modules/base/processors/sequencesource.h>

namespace inviwo {

std::expected<std::filesystem::path, std::string_view> util::getFirstFileInFolder(
    const std::filesystem::path& folder, std::string_view filter) {

    if (!std::filesystem::is_directory(folder)) {
        static constexpr std::string_view noFolderReason{"Not a folder"};
        return std::unexpected(noFolderReason);
    }

    auto view = std::filesystem::directory_iterator{folder} |
                std::views::filter([](const std::filesystem::directory_entry& entry) {
                    return entry.is_regular_file();
                }) |
                std::views::transform(
                    [](const std::filesystem::directory_entry& entry) { return entry.path(); });

    if (filter.empty()) {
        if (std::ranges::begin(view) != std::ranges::end(view)) {
            return *std::ranges::begin(view);
        } else {
            static constexpr std::string_view emptyFolderReason{"Empty folder"};
            return std::unexpected(emptyFolderReason);
        }
    } else {
        try {
            const auto fv = filter | views::codePoints;
            const auto re = std::regex{fv.begin(), fv.end()};
            auto matches = view | std::views::filter([&](const std::filesystem::path& path) {
                               auto pv = path.native() | views::codePoints;
                               return regex_match(pv.begin(), pv.end(), re);
                           });
            if (std::ranges::begin(matches) != std::ranges::end(matches)) {
                return *std::ranges::begin(matches);
            } else {
                static constexpr std::string_view noMatchesReason{"No matching files"};
                return std::unexpected(noMatchesReason);
            }
        } catch (const std::regex_error&) {
            static constexpr std::string_view invalidFilterReason{"Invalid filter"};
            return std::unexpected(invalidFilterReason);
        }
    }
}

}  // namespace inviwo
