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

namespace {

auto filteredFilesView(const std::filesystem::path& folder, std::string_view include,
                       std::string_view exclude) {

    std::optional<std::wregex> includeRe = std::nullopt;
    std::optional<std::wregex> excludeRe = std::nullopt;

    if (!include.empty()) {
        const auto v = include | views::codePoints | views::wChars | std::ranges::to<std::vector>();
        includeRe = std::wregex{v.begin(), v.end()};
    }
    if (!exclude.empty()) {
        const auto v = exclude | views::codePoints | views::wChars | std::ranges::to<std::vector>();
        excludeRe.emplace(v.begin(), v.end());
    }

    return std::filesystem::directory_iterator{folder} |
           std::views::filter([](const std::filesystem::directory_entry& entry) {
               return entry.is_regular_file();
           }) |
           std::views::transform(
               [](const std::filesystem::directory_entry& entry) { return entry.path(); }) |
           std::views::filter([includeRe, excludeRe](const std::filesystem::path& path) {
               auto pv = path.native() | views::codePoints | views::wChars;
               static_assert(std::forward_iterator<decltype(pv.begin())>);
               static_assert(std::forward_iterator<decltype(pv.end())>);

               bool included = true;

               if (includeRe && !regex_search(pv.begin(), pv.end(), *includeRe)) {
                   included &= false;
               }
               if (excludeRe && regex_search(pv.begin(), pv.end(), *excludeRe)) {
                   included &= false;
               }
               if (std::ranges::ends_with(path.native() | views::codePoints,
                                          std::string_view{".DS_Store"} | views::codePoints)) {
                   included &= false;
               }
               return included;
           });
}

std::expected<decltype(filteredFilesView(std::filesystem::path{}, std::string_view{},
                                         std::string_view{})),
              std::string_view>
getFolderView(const std::filesystem::path& folder, std::string_view include,
              std::string_view exclude) {

    if (!std::filesystem::is_directory(folder)) {
        static constexpr std::string_view noFolderReason{"Not a folder"};
        return std::unexpected(noFolderReason);
    }

    try {
        auto view = filteredFilesView(folder, include, exclude);
        if (std::ranges::begin(view) != std::ranges::end(view)) {
            return view;
        } else {
            static constexpr std::string_view noMatchesReason{"No matching files"};
            return std::unexpected(noMatchesReason);
        }
    } catch (const std::regex_error&) {
        static constexpr std::string_view invalidFilterReason{"Invalid filter"};
        return std::unexpected(invalidFilterReason);
    }
}

}  // namespace

std::expected<std::vector<std::filesystem::path>, std::string_view> util::getFilesInFolder(
    const std::filesystem::path& folder, std::string_view include, std::string_view exclude) {

    return getFolderView(folder, include, exclude)
        .transform([](auto value) -> std::vector<std::filesystem::path> {
            auto files = value | std::ranges::to<std::vector>();
            std::ranges::sort(files);
            return files;
        });
}

std::expected<std::filesystem::path, std::string_view> util::getFirstFileInFolder(
    const std::filesystem::path& folder, std::string_view include, std::string_view exclude) {

    return getFolderView(folder, include, exclude)
        .transform([](auto value) -> std::filesystem::path { return std::ranges::min(value); });
}

}  // namespace inviwo
