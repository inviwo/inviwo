/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwo/meta/paths.hpp>
#include <inviwo/meta/cmake/cmakefile.hpp>

#include <array>
#include <algorithm>
#include <fstream>
#include <string>

namespace inviwo::meta {

bool isInviwoDir(const std::filesystem::path& inviwoPath) {
    using path = std::filesystem::path;
    const std::array dirs = {path{"README.md"},
                             path{"CHANGELOG.md"},
                             path{"modules"} / path{"base"},
                             path{"include"} / path{"inviwo"},
                             path{"tools"} / path{"templates"},
                             path{"src"} / path{"core"}};

    return std::all_of(dirs.begin(), dirs.end(),
                       [&](const auto& dir) { return std::filesystem::exists(inviwoPath / dir); });
}

std::optional<std::filesystem::path> findInviwoUsingCMakeCache(
    std::vector<std::filesystem::path> guesses) {

    using path = std::filesystem::path;

    const std::string_view CMakeCache = "CMakeCache.txt";
    const std::string_view pattern = "INVIWOCOREMODULE_base";
    const std::string_view prefix = "INVIWOCOREMODULE_base:INTERNAL=";

    while (!guesses.empty()) {
        for (const auto& guess : guesses) {
            if (std::filesystem::exists(guess / CMakeCache)) {
                std::ifstream cmc{guess / CMakeCache};

                std::string line;
                while (std::getline(cmc, line)) {
                    if (line.size() > prefix.size() &&
                        std::string_view(line.data(), pattern.size()) == pattern) {
                        const auto ivwSrcPath = path{std::string_view{line.data() + prefix.size(),
                                                                      line.size() - prefix.size()}};
                        const auto ivwPath = ivwSrcPath.parent_path();
                        if (isInviwoDir(ivwPath)) {
                            return ivwPath;
                        }
                    }
                }
            }
        }
        guesses.erase(std::remove_if(guesses.begin(), guesses.end(),
                                     [](const auto& p) { return p == p.parent_path(); }),
                      guesses.end());
        for (auto& p : guesses) {
            p = p.parent_path();
        }
    }
    return {};
}

std::optional<std::filesystem::path> findInviwoPath(std::vector<std::filesystem::path> guesses) {
    if (auto path = findInviwoUsingCMakeCache(guesses)) {
        return path;
    }

    while (!guesses.empty()) {
        for (const auto& guess : guesses) {
            if (isInviwoDir(guess)) return guess;
            for (const auto& p : std::filesystem::recursive_directory_iterator(guess)) {
                if (p.is_directory() && isInviwoDir(p)) return p;
            }
        }

        guesses.erase(std::remove_if(guesses.begin(), guesses.end(),
                                     [](const auto& p) { return p == p.parent_path(); }),
                      guesses.end());

        for (auto& p : guesses) {
            p = p.parent_path();
        }
    }
    return {};
}

}  // namespace inviwo::meta
