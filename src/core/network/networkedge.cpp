/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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
NetworkEdge::NetworkEdge(std::string asrc, std::string adst)
    : src{std::move(asrc)}, dst{std::move(adst)} {}

NetworkEdge::NetworkEdge(const PropertyLink& link)
    : src{link.getSource()->getPathStr()}
    , dst{link.getDestination()->getPathStr()} {}

NetworkEdge::NetworkEdge(const PortConnection& connection)
    : src{connection.getOutport()->getPathStr()}
    , dst{connection.getInport()->getPathStr()} {}

PortConnection NetworkEdge::toConnection(const ProcessorNetwork& net) const {
    auto outport = net.getOutport(src);
    auto inport = net.getInport(dst);

    constexpr std::string_view err =
        "Could not create Connection from:\nOutport '{}'\nto\nInport '{}'\n{}";
    if (!outport && !inport) {
        const auto message = fmt::format(err, src, dst, "Outport and Inport not found.");
        throw SerializationException(message, IVW_CONTEXT, "Connection");
    } else if (!outport) {
        const auto message = fmt::format(err, src, dst, "Outport not found.");
        throw SerializationException(message, IVW_CONTEXT, "Connection");
    } else if (!inport) {
        const auto message = fmt::format(err, src, dst, "Inport not found.");
        throw SerializationException(message, IVW_CONTEXT, "Connection");
    }

    return {outport, inport};
}

PropertyLink NetworkEdge::toLink(const ProcessorNetwork& net) const {
    auto sprop = net.getProperty(src);
    auto dprop = net.getProperty(dst);

    constexpr std::string_view err =
        "Could not create Property Link from:\nSource '{}'\nto\nDestination '{}'\n{}";
    if (!sprop && !dprop) {
        const auto message =
            fmt::format(err, src, dst, "Source and destination properties not found.");
        throw SerializationException(message, IVW_CONTEXT, "PropertyLink");
    } else if (!sprop) {
        const auto message = fmt::format(err, src, dst, "Source property not found.");
        throw SerializationException(message, IVW_CONTEXT, "PropertyLink");

    } else if (!dprop) {
        const auto message = fmt::format(err, src, dst, "Destination property not found.");
        throw SerializationException(message, IVW_CONTEXT, "PropertyLink");
    }

    return {sprop, dprop};
}

void NetworkEdge::updateProcessorID(const std::unordered_map<std::string, std::string>& map) {
    {
        const auto len = src.find_first_of('.');
        std::string id = src.substr(0, len);
        auto it = map.find(id);
        if (it != map.end()) {
            src = fmt::format("{}.{}", it->second, src.substr(len));
        }
    }
    {
        const auto len = dst.find_first_of('.');
        const std::string id = dst.substr(0, len);
        auto it = map.find(id);
        if (it != map.end()) {
            dst = fmt::format("{}.{}", it->second, dst.substr(len));
        }
    }
}

void NetworkEdge::serialize(Serializer& s) const {
    s.serialize("src", src, SerializationTarget::Attribute);
    s.serialize("dst", dst, SerializationTarget::Attribute);
}

void NetworkEdge::deserialize(Deserializer& d) {
    d.deserialize("src", src, SerializationTarget::Attribute);
    d.deserialize("dst", dst, SerializationTarget::Attribute);
}
}  // namespace inviwo
