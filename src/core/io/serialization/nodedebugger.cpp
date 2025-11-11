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

#include <inviwo/core/io/serialization/nodedebugger.h>
#include <inviwo/core/io/serialization/serializeconstants.h>
#include <ticpp/tinyxml.h>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <ranges>

namespace inviwo::deserializer {

std::string format_as(const Node& node) {
    if (node.identifier && node.type) {
        return fmt::format("{}: \"{}\" of class {}", node.key, *node.identifier, *node.type);
    } else if (node.identifier) {
        return fmt::format("{}: \"{}\"", node.key, *node.identifier);
    } else if (node.type) {
        return fmt::format("{} of class {}", node.key, *node.type);
    } else {
        return node.key;
    }
}

Node getNode(const TiXmlElement* node) {
    return {.key = std::string{node->Value()},
            .identifier = node->Attribute("identifier").transform([](std::string_view str) {
                return std::string{str};
            }),
            .type = node->Attribute(SerializeConstants::TypeAttribute)
                        .transform([](std::string_view str) { return std::string{str}; })};
}

std::vector<Node> getStack(const TiXmlElement* elem) {
    std::vector<Node> stack;
    while (elem) {
        stack.emplace_back(getNode(elem));
        elem = elem->Parent()->ToElement();
    }
    return stack;
}

std::vector<std::string> getPath(const std::vector<Node>& stack) {
    std::vector<std::string> path;
    for (const auto& elem : stack | std::views::reverse) {
        if (elem.identifier) path.push_back(*elem.identifier);
    }
    return path;
}

std::string getDescription(const std::vector<Node>& stack) {
    return fmt::format("{}", fmt::join(stack, " in\n   "));
}

}  // namespace inviwo::deserializer
