/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2024 Inviwo Foundation
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

#include <inviwo/core/network/networkedge.h>

#include <inviwo/core/network/portconnection.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/links/propertylink.h>
#include <inviwo/core/io/serialization/serialization.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/io/serialization/serializationexception.h>
#include <inviwo/core/util/stringconversion.h>

#include <fmt/format.h>

namespace inviwo {

NetworkEdge::NetworkEdge(std::string src, std::string dst)
    : srcPath{std::move(src)}, dstPath{std::move(dst)} {}

NetworkEdge::NetworkEdge(const PropertyLink& link)
    : srcPath{link.getSource()->getPath()}, dstPath{link.getDestination()->getPath()} {}

NetworkEdge::NetworkEdge(const PortConnection& connection)
    : srcPath{connection.getOutport()->getPath()}, dstPath{connection.getInport()->getPath()} {}

PortConnection NetworkEdge::toConnection(const ProcessorNetwork& net) const {
    auto outport = net.getOutport(srcPath);
    auto inport = net.getInport(dstPath);

    constexpr std::string_view err =
        "Could not create Connection from:\nOutport '{}'\nto\nInport '{}'\n{}";
    if (!outport && !inport) {
        const auto message = fmt::format(err, srcPath, dstPath, "Outport and Inport not found.");
        throw SerializationException(message, IVW_CONTEXT, "Connection");
    } else if (!outport) {
        const auto message = fmt::format(err, srcPath, dstPath, "Outport not found.");
        throw SerializationException(message, IVW_CONTEXT, "Connection");
    } else if (!inport) {
        const auto message = fmt::format(err, srcPath, dstPath, "Inport not found.");
        throw SerializationException(message, IVW_CONTEXT, "Connection");
    }

    return {outport, inport};
}

PropertyLink NetworkEdge::toLink(const ProcessorNetwork& net) const {
    auto sprop = net.getProperty(srcPath);
    auto dprop = net.getProperty(dstPath);

    constexpr std::string_view err =
        "Could not create Property Link from:\nSource '{}'\nto\nDestination '{}'\n{}";
    if (!sprop && !dprop) {
        const auto message =
            fmt::format(err, srcPath, dstPath, "Source and destination properties not found.");
        throw SerializationException(message, IVW_CONTEXT, "PropertyLink");
    } else if (!sprop) {
        const auto message = fmt::format(err, srcPath, dstPath, "Source property not found.");
        throw SerializationException(message, IVW_CONTEXT, "PropertyLink");

    } else if (!dprop) {
        const auto message = fmt::format(err, srcPath, dstPath, "Destination property not found.");
        throw SerializationException(message, IVW_CONTEXT, "PropertyLink");
    }

    return {sprop, dprop};
}

void NetworkEdge::updateSrcProcessorID(const std::map<std::string, std::string, std::less<>>& map) {
    const auto [pid, path] = util::splitByFirst(srcPath, ".");
    const auto it = map.find(pid);
    if (it != map.end()) {
        srcPath = fmt::format("{}.{}", it->second, path);
    }
}

void NetworkEdge::updateDstProcessorID(const std::map<std::string, std::string, std::less<>>& map) {
    const auto [pid, path] = util::splitByFirst(dstPath, ".");
    const auto it = map.find(pid);
    if (it != map.end()) {
        dstPath = fmt::format("{}.{}", it->second, path);
    }
}

void NetworkEdge::updateProcessorID(const std::map<std::string, std::string, std::less<>>& map) {
    updateSrcProcessorID(map);
    updateDstProcessorID(map);
}

void NetworkEdge::serialize(Serializer& s) const {
    s.serialize("src", srcPath, SerializationTarget::Attribute);
    s.serialize("dst", dstPath, SerializationTarget::Attribute);
}

void NetworkEdge::deserialize(Deserializer& d) {
    d.deserialize("src", srcPath, SerializationTarget::Attribute);
    d.deserialize("dst", dstPath, SerializationTarget::Attribute);
}
}  // namespace inviwo
