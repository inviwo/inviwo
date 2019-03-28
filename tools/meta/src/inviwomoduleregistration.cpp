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

#include <inviwo/meta/inviwomoduleregistration.hpp>
#include <inviwo/meta/util.hpp>

#include <fstream>
#include <streambuf>
#include <regex>
#include <sstream>
#include <vector>

#include <fmt/format.h>

namespace inviwo::meta {

InviwoModuleRegistration::InviwoModuleRegistration(const std::filesystem::path& aModulecpp)
    : modulecpp{aModulecpp} {
    std::ifstream ifs{modulecpp};
    if (!ifs) throw util::makeError("Unable to open module file '{}'", modulecpp.generic_string());
    file = std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

void InviwoModuleRegistration::addInclude(const std::filesystem::path& incPath) {
    const auto reInclude = std::regex(R"(#include\s* <([\w/.]*)>)");
    const std::vector incMatches(std::sregex_iterator(file.begin(), file.end(), reInclude),
                                 std::sregex_iterator());

    if (incMatches.empty())
        throw util::makeError("Could not find any includes statments in '{}'",
                              modulecpp.generic_string());

    if (std::find_if(incMatches.begin(), incMatches.end(), [&](auto& m) {
            return m.str(1) == incPath.generic_string();
        }) != incMatches.end()) {
        throw util::makeError("Error: Include '{}' already added to '{}", incPath.generic_string(),
                              modulecpp.generic_string());
    }

    const auto it =
        std::lower_bound(incMatches.begin(), incMatches.end(), incPath.generic_string(),
                         [](const auto& it, const auto& str) { return it.str(1) < str; });

    std::stringstream res;
    if (it != incMatches.end()) {
        const size_t pos = it->position();
        res << std::string_view{file.data(), pos};
        res << "#include <" << incPath.generic_string() << ">\n";
        res << std::string_view{file.data() + pos, file.size() - pos};
    } else {
        const size_t pos = incMatches.back().position() + incMatches.back().length();
        res << std::string_view{file.data(), pos};
        res << "\n#include <" << incPath.generic_string() << ">";
        res << std::string_view{file.data() + pos, file.size() - pos};
    }
    file = std::move(res).str();
}

void InviwoModuleRegistration::registerProcessor(std::string_view name,
                                                 const std::filesystem::path& incPath) {
    addInclude(incPath);

    const auto mProcessor = std::regex(
        R"(([ \t]*(?:\/\/)?[ \t]*registerProcessor<([a-zA-Z0-9_:\<\>,]+)>\(\);.*)(?=\n\r|\n))");
    const std::vector procMatches(std::sregex_iterator(file.begin(), file.end(), mProcessor),
                                  std::sregex_iterator());

    std::stringstream res;
    if (procMatches.empty()) {
        const auto reConstr =
            std::regex(R"(\s*\w+Module::\w+Module\(\s*InviwoApplication\s*\*\s*\w+\s*\))"
                       R"(\s*:\s*InviwoModule\(\s*\w+\s*,\s*"\w+"\s*\)\s*\{)");
        std::smatch constrMatch;
        if (!std::regex_search(file, constrMatch, reConstr)) {
            throw std::runtime_error("Module constructor not found!");
        }
        const size_t pos = constrMatch.position() + constrMatch.length();
        res << std::string_view{file.data(), pos};
        res << "\n    registerProcessor<" << name << ">();";
        res << std::string_view{file.data() + pos, file.size() - pos};
    } else {
        if (std::find_if(procMatches.begin(), procMatches.end(),
                         [&](auto& m) { return m.str(2) == name; }) != procMatches.end()) {
            throw util::makeError("Error: Processor '{}' already registered in '{}", name,
                                  modulecpp.generic_string());
        }

        auto it = std::lower_bound(procMatches.begin(), procMatches.end(), name,
                                   [](const auto& it, const auto& str) { return it.str(2) < str; });
        if (it != procMatches.end()) {
            const size_t pos = it->position();
            res << std::string_view{file.data(), pos};
            res << "    registerProcessor<" << name << ">();\n";
            res << std::string_view{file.data() + pos, file.size() - pos};
        } else {
            const size_t pos = procMatches.back().position(1) + procMatches.back().length(1);
            res << std::string_view{file.data(), pos};
            res << "\n    registerProcessor<" << name << ">();";
            res << std::string_view{file.data() + pos, file.size() - pos};
        }
    }
    file = res.str();
}

InviwoModuleRegistration::~InviwoModuleRegistration() {
    if (!std::uncaught_exceptions()) {
        std::ofstream ofs{modulecpp};
        ofs << file;
    }
}

}  // namespace inviwo::meta
