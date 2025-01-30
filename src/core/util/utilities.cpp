/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/util/stringconversion.h>

#include <inviwo/core/properties/property.h>
#include <fmt/base.h>
#include <fmt/std.h>
#include <ranges>
#include <filesystem>

namespace inviwo {
namespace util {

void saveNetwork(ProcessorNetwork* network, std::string_view filename) {
    try {
        Serializer xmlSerializer(filename);
        network->serialize(xmlSerializer);
        xmlSerializer.writeFile();
    } catch (SerializationException& e) {
        log::exception(e, "Unable to save network {} due to {}", filename, e.getMessage());
    }
}

void saveAllCanvases(ProcessorNetwork* network, const std::filesystem::path& dir,
                     std::string_view name, std::string_view ext, bool onlyActiveCanvases) {

    // Get all canvases, possibly only the active ones. We need their count below.
    auto allCanvases = network->getProcessorsByType<inviwo::CanvasProcessor>();
    std::vector<CanvasProcessor*> allConsideredCanvases;
    if (onlyActiveCanvases) {
        allConsideredCanvases.reserve(allCanvases.size());
        for (auto cp : allCanvases) {
            if (cp->isSink()) allConsideredCanvases.push_back(cp);
        }
    } else {
        allConsideredCanvases = allCanvases;
    }

    // Save them
    int i = 0;
    for (auto cp : allConsideredCanvases) {
        if (!cp->isValid() || !cp->isReady()) {
            std::ostringstream msg;
            log::error(
                "Canvas is not ready or not valid, no image saved\n"
                "    Display Name: {}\n"
                "    Identifier: {}",
                cp->getDisplayName(), cp->getIdentifier());

        } else {

            StrBuffer filepath;
            if (name.empty()) {
                filepath.append("{}", cp->getIdentifier());
            } else if (name.find("UPN") != std::string::npos) {
                auto [before, after] = util::splitByFirst(name, "UPN");
                filepath.append("{}{}{}", before, cp->getIdentifier(), after);
            } else if (allConsideredCanvases.size() > 1) {
                filepath.append("{}{}", name, i + 1);
            } else {
                filepath.append("{}", name);
            }

            if (!ext.empty() && ext[0] != '.') {
                filepath.append(".{}", ext);
            } else {
                filepath.append("{}", ext);
            }

            auto path = dir / filepath.view();

            log::info("Saving canvas to: {}", path);
            cp->saveImageLayer(path);
        }
        i++;
    }
}

bool isValidIdentifierCharacter(char c, std::string_view extra) {
    return (std::isalnum(c) || c == '_' || util::contains(extra, c));
}

void validateIdentifier(std::string_view identifier, std::string_view type, SourceContext context) {
    if (identifier.empty()) {
        throw Exception(fmt::format("{} identifiers must not be empty", type), context);
    }

    if (!std::isalpha(identifier[0]) && !identifier.starts_with('_')) {
        throw Exception(fmt::format("{} identifiers must start with a letter or underscore: '{}'",
                                    type, identifier),
                        context);
    }

    auto validChar = [](char c) -> bool { return c == '_' || std::isalnum(c); };
    auto view = identifier | std::views::drop(1) | std::views::drop_while(validChar);
    if (view.begin() != view.end()) {
        throw Exception(
            fmt::format("{} identifiers are not allowed to contain \"{}\". Found in \"{}\"", type,
                        view.front(), identifier),
            context);
    }
}

std::string findUniqueIdentifier(std::string_view identifier,
                                 std::function<bool(std::string_view)> isUnique,
                                 std::string_view sep) {

    int i = 2;

    const auto pos = identifier.find_last_not_of("0123456789");
    const std::string_view baseIdentifier = util::trim(identifier.substr(0, pos + 1));

    if ((pos + 1) < identifier.size()) {
        const std::string_view numstr = identifier.substr(pos + 1);
        i = stringTo<int>(numstr);
    }

    std::string newIdentifier{identifier};
    while (!isUnique(newIdentifier)) {
        newIdentifier = fmt::format("{}{}{}", baseIdentifier, sep, i++);
    }
    return newIdentifier;
}

std::string cleanIdentifier(std::string_view identifier, std::string_view extra) {
    std::string str{identifier};
    std::replace_if(
        str.begin(), str.end(),
        [&](char c) { return !(util::contains(extra, c) || std::isalnum(c) || c == '_'); }, ' ');
    std::erase_if(str, [s = false](char c) mutable {
        if (s && c == ' ') return true;
        s = c == ' ';
        return false;
    });
    return str;
}

std::string stripModuleFileNameDecoration(const std::filesystem::path& filePath) {
    auto fileNameWithoutExtension = filePath.stem().string();
#if defined(WIN32)
    auto decoration1 = std::string_view("inviwo-module-");
    auto decoration2 = std::string_view("inviwo-");
#else
    auto decoration1 = std::string_view("libinviwo-module-");
    auto decoration2 = std::string_view("libinviwo-");
#endif
    auto inviwoModulePos = fileNameWithoutExtension.find(decoration1);
    if (inviwoModulePos == std::string::npos) {
        inviwoModulePos = fileNameWithoutExtension.find(decoration2);
        if (inviwoModulePos == std::string::npos) {
            inviwoModulePos = 0;
        } else {
            inviwoModulePos = decoration2.size();
        }
    } else {
        inviwoModulePos = decoration1.size();
    }
    auto len = fileNameWithoutExtension.size() - inviwoModulePos;
#ifdef IVW_DEBUG
    // Remove debug ending "d" at end of file name
    len -= 1;
#endif
    auto moduleName = fileNameWithoutExtension.substr(inviwoModulePos, len);
    return moduleName;
}

std::string stripIdentifier(std::string_view identifier) {
    // Valid identifier: [a-zA-Z_][a-zA-Z0-9_]*
    auto firstCharValid = [](char c) -> bool { return (c == '_' || std::isalpha(c)); };
    auto invalidChar = [](char c) -> bool { return !(c == '_' || std::isalnum(c)); };

    std::string stripped{identifier};
    while (!stripped.empty()) {
        const auto& c = stripped[0];
        if (firstCharValid(c)) {
            break;
        }
        if (std::isdigit(c)) {  // prepend an underscore if first char is a digit
            stripped = "_" + stripped;
            break;
        }
        stripped = stripped.substr(1);
    }

    std::erase_if(stripped, invalidChar);
    if (stripped.empty()) {
        stripped = "_";
    }
    return stripped;
}

namespace detail {

void Shower::operator()(Property& p) { p.setVisible(true); }
void Hideer::operator()(Property& p) { p.setVisible(false); }

void Shower::operator()(ProcessorWidget& p) { p.setVisible(true); }
void Hideer::operator()(ProcessorWidget& p) { p.setVisible(false); }

}  // namespace detail

}  // namespace util
}  // namespace inviwo
