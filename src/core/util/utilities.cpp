/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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
#include <fmt/format.h>

namespace inviwo {
namespace util {

void saveNetwork(ProcessorNetwork* network, std::string_view filename) {
    try {
        Serializer xmlSerializer(filename);
        network->serialize(xmlSerializer);
        xmlSerializer.writeFile();
    } catch (SerializationException& exception) {
        util::log(
            exception.getContext(),
            fmt::format("Unable to save network {} due to {}", filename, exception.getMessage()),
            LogLevel::Error);
    }
}

void saveAllCanvases(ProcessorNetwork* network, std::string_view dir, std::string_view name,
                     std::string_view ext, bool onlyActiveCanvases) {

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
            msg << "Canvas is not ready or not valid, no image saved" << std::endl;
            msg << "    Display Name: " << cp->getDisplayName() << std::endl;
            msg << "    Identifier: " << cp->getIdentifier() << std::endl;
            LogErrorCustom("util::saveAllCanvases", msg.str());
        } else {

            StrBuffer filepath;
            filepath.append("{}/", dir);

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

            LogInfoCustom("util::saveAllCanvases", "Saving canvas to: " << filepath.view());
            cp->saveImageLayer(filepath.view());
        }
        i++;
    }
}

bool isValidIdentifierCharacter(char c, std::string_view extra) {
    return (std::isalnum(c) || c == '_' || c == '-' || util::contains(extra, c));
}

void validateIdentifier(std::string_view identifier, std::string_view type,
                        ExceptionContext context, std::string_view extra) {
    for (const auto& c : identifier) {
        if (c == 0) return;
        if (!(c >= -1) || !isValidIdentifierCharacter(c, extra)) {
            throw Exception(
                fmt::format("{} identifiers are not allowed to contain \"{}\". Found in \"{}\"",
                            type, c, identifier),
                context);
        }
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
        str.begin(), str.end(), [&](char c) { return !util::isValidIdentifierCharacter(c, extra); },
        ' ');
    util::erase_remove_if(str, [s = false](char c) mutable {
        if (s && c == ' ') return true;
        s = c == ' ';
        return false;
    });
    return str;
}

std::string stripModuleFileNameDecoration(std::string_view filePath) {
    auto fileNameWithoutExtension = filesystem::getFileNameWithoutExtension(filePath);
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
    std::string stripped{identifier};
    // What we allow: [a-zA-Z_][a-zA-Z0-9_]*
    auto testFirst = [](unsigned char c) -> bool { return !(c == '_' || std::isalpha(c)); };

    auto testRest = [](unsigned char c) -> bool { return !(c == '_' || std::isalnum(c)); };

    while (stripped.size() > 0) {
        const auto& c = stripped[0];
        if (!testFirst(c)) {
            break;
        }
        if (std::isdigit(c)) {  // prepend an underscore if first char is a digit
            stripped = "_" + stripped;
            break;
        }
        stripped = stripped.substr(1);
    }

    util::erase_remove_if(stripped, testRest);
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
