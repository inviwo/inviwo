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

auto filteredFilesView(const std::filesystem::path& folder, std::optional<std::string_view> include,
                       std::optional<std::string_view> exclude) {

    static constexpr auto makeRegex = [](std::string_view pattern) {
        const auto v = pattern | views::codePoints | views::wChars | std::ranges::to<std::vector>();
        return std::wregex{v.begin(), v.end()};
    };

    static constexpr auto search = [](auto&& view, const std::wregex& regex) {
        return regex_search(view.begin(), view.end(), regex);
    };

    static constexpr auto ends_with = [](const std::filesystem::path& path, std::string_view str) {
        return std::ranges::ends_with(path.native() | views::codePoints, str | views::codePoints);
    };

    return std::filesystem::directory_iterator{folder} |
           std::views::filter([](const std::filesystem::directory_entry& entry) {
               return entry.is_regular_file();
           }) |
           std::views::transform(
               [](const std::filesystem::directory_entry& entry) { return entry.path(); }) |
           std::views::filter(
               [includeRe = include.transform(makeRegex),
                excludeRe = exclude.transform(makeRegex)](const std::filesystem::path& path) {
                   auto pv = path.native() | views::codePoints | views::wChars;

                   return (!includeRe || search(pv, *includeRe)) &&
                          (!excludeRe || !search(pv, *excludeRe)) &&
                          !ends_with(path, std::string_view{".DS_Store"});
               });
}

using FilesView = decltype(filteredFilesView(
    std::filesystem::path{}, std::optional<std::string_view>{}, std::optional<std::string_view>{}));

auto getFolderView(const std::filesystem::path& folder, std::optional<std::string_view> include,
                   std::optional<std::string_view> exclude)
    -> std::expected<FilesView, std::string_view> {

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

auto util::getFilesInFolder(const std::filesystem::path& folder,
                            std::optional<std::string_view> include,
                            std::optional<std::string_view> exclude)
    -> std::expected<std::vector<std::filesystem::path>, std::string_view> {

    return getFolderView(folder, include, exclude)
        .transform([](auto value) -> std::vector<std::filesystem::path> {
            auto files = value | std::ranges::to<std::vector>();
            std::ranges::sort(files);
            return files;
        });
}

auto util::getFileInFolder(const std::filesystem::path& folder,
                           std::optional<std::string_view> include,
                           std::optional<std::string_view> exclude)
    -> std::expected<std::filesystem::path, std::string_view> {

    return getFolderView(folder, include, exclude)
        .transform([](auto value) -> std::filesystem::path { return *std::ranges::begin(value); });
}

}  // namespace inviwo
