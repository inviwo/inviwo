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

#pragma once

#include <inviwo/meta/inviwometadefine.hpp>
#include <inviwo/meta/inviwomodule.hpp>
#include <inviwo/meta/filetemplates.hpp>

#include <filesystem>
#include <optional>
#include <string_view>
#include <string>
#include <iostream>

#include <nlohmann/json.hpp>
#include <fmt/format.h>

namespace inviwo::meta {

namespace json = ::nlohmann;

class INVIWO_META_API Creator {
public:
    struct Options {
        const bool verbose;
        const bool dryrun;
        const bool force;
        std::ostream& log;
    };

    Creator(const std::filesystem::path& inviwoRepo,
            std::optional<std::filesystem::path> templateDir = {},
            std::optional<std::filesystem::path> configFile = {},
            Options opts = {false, false, false, std::cout});

    void createModule(const std::filesystem::path& modulePath, std::string_view org) const;
    void createFile(const std::filesystem::path& filePath) const;
    void createProcessor(const std::filesystem::path& processorPath) const;
    void createTest(const std::filesystem::path& testPath) const;

    /**
     * Update a module to use include and src folders.
     * Will move all .h file into the include/<org>/<module> sub folder
     * and all .cpp into the src folder.
     * except for files under /ext, /tests, or paths excluded be the given filters.
     */
    void updateModule(const std::filesystem::path& modulePath, std::string_view org,
                      const std::vector<std::string>& filters) const;

    Options opts;

private:
    void generate(const std::filesystem::path& filePath, std::string_view tag) const;
    void generate(InviwoModule& im, const std::filesystem::path& filePath,
                  std::string_view tag) const;

    void log(std::string_view message) const;
    void log(std::string_view key, std::string_view message) const;

    template <typename S, typename... Args>
    void logf(S&& s, Args&&... args) const {
        log(fmt::format(std::forward<S>(s), std::forward<Args>(args)...));
    }

    json::json createSettings(const InviwoModule& module, std::string_view fileName) const;

    std::filesystem::path inviwoRepo_;
    std::filesystem::path templateDir_;
    std::filesystem::path configFile_;
    FileTemplates templates_;
};

}  // namespace inviwo::meta
