/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/processors/processorwidget.h>

#include <inviwo/core/properties/property.h>

namespace inviwo {
namespace util {

void saveNetwork(ProcessorNetwork* network, std::string filename) {
    try {
        Serializer xmlSerializer(filename);
        network->serialize(xmlSerializer);
        xmlSerializer.writeFile();
    } catch (SerializationException& exception) {
        util::log(exception.getContext(),
                  "Unable to save network " + filename + " due to " + exception.getMessage(),
                  LogLevel::Error);
    }
}

void saveAllCanvases(ProcessorNetwork* network, const std::string& dir, const std::string& name,
                     const std::string& ext, bool onlyActiveCanvases) {

    // Get all canvases, possibly only the active ones. We need their count below.
    std::vector<CanvasProcessor*> allCanvases =
        network->getProcessorsByType<inviwo::CanvasProcessor>();
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
            std::stringstream ss;
            ss << dir << "/";

            if (name == "") {
                ss << cp->getIdentifier();
            } else if (name.find("UPN") != std::string::npos) {
                std::string tmp = name;
                replaceInString(tmp, "UPN", cp->getIdentifier());
                ss << tmp;
            } else {
                ss << name << ((allConsideredCanvases.size() > 1) ? std::to_string(i + 1) : "");
            }
            ss << ((ext.size() && ext[0] != '.') ? "." : "") << ext;

            LogInfoCustom("util::saveAllCanvases", "Saving canvas to: " + ss.str());
            cp->saveImageLayer(ss.str());
        }
        i++;
    }
}

bool isValidIdentifierCharacter(char c, const std::string& extra) {
    return (std::isalnum(c) || c == '_' || c == '-' || util::contains(extra, c));
}

void validateIdentifier(const std::string& identifier, const std::string& type,
                        ExceptionContext context, const std::string& extra) {
    for (const auto& c : identifier) {
        if (!(c >= -1) || !isValidIdentifierCharacter(c, extra)) {
            throw Exception(type + " identifiers are not allowed to contain \"" + c +
                                "\". Found in \"" + identifier + "\"",
                            context);
        }
    }
}

std::string findUniqueIdentifier(const std::string& identifier,
                                 std::function<bool(const std::string&)> isUnique,
                                 const std::string& sep) {

    int i = 2;
    std::string newIdentifier = identifier;
    auto it = std::find_if(identifier.rbegin(), identifier.rend(),
                           [](char c) { return !std::isdigit(c); });
    std::string baseIdentifier = trim(std::string{identifier.begin(), it.base()});
    std::string number(it.base(), identifier.end());
    if (!number.empty()) i = std::stoi(number);

    while (!isUnique(newIdentifier)) {
        newIdentifier = baseIdentifier + sep + toString(i++);
    }
    return newIdentifier;
}

std::string cleanIdentifier(const std::string& identifier, const std::string& extra) {
    std::string str{identifier};
    std::replace_if(str.begin(), str.end(),
                    [&](char c) { return !util::isValidIdentifierCharacter(c, extra); }, ' ');
    util::erase_remove_if(str, [s = false](char c) mutable {
        if (s && c == ' ') return true;
        s = c == ' ';
        return false;
    });
    return str;
}

std::string stripModuleFileNameDecoration(std::string filePath) {
    auto fileNameWithoutExtension = filesystem::getFileNameWithoutExtension(filePath);
#if defined(WIN32)
    auto decoration1 = std::string("inviwo-module-");
    auto decoration2 = std::string("inviwo-");
#else
    auto decoration1 = std::string("libinviwo-module-");
    auto decoration2 = std::string("libinviwo-");
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
#ifdef DEBUG
    // Remove debug ending "d" at end of file name
    len -= 1;
#endif
    auto moduleName = fileNameWithoutExtension.substr(inviwoModulePos, len);
    return moduleName;
}

std::string stripIdentifier(std::string identifier) {
    // What we allow: [a-zA-Z_][a-zA-Z0-9_]*
    auto testFirst = [](unsigned char c) -> bool { return !(c == '_' || std::isalpha(c)); };

    auto testRest = [](unsigned char c) -> bool { return !(c == '_' || std::isalnum(c)); };

    while (identifier.size() > 0) {
        const auto& c = identifier[0];
        if (!testFirst(c)) {
            break;
        }
        if (std::isdigit(c)) {  // prepend an underscore if first char is a digit
            identifier = "_" + identifier;
            break;
        }
        identifier = identifier.substr(1);
    }

    util::erase_remove_if(identifier, testRest);
    return identifier;
}

namespace detail {

void Shower::operator()(Property& p) { p.setVisible(true); }
void Hideer::operator()(Property& p) { p.setVisible(false); }

void Shower::operator()(ProcessorWidget& p) { p.setVisible(true); }
void Hideer::operator()(ProcessorWidget& p) { p.setVisible(false); }

}  // namespace detail

}  // namespace util
}  // namespace inviwo
