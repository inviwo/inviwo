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
#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/io/serialization/serializable.h>

#include <string>
#include <map>

namespace inviwo {

class PropertyLink;
class PortConnection;
class ProcessorNetwork;

/**
 * Represents a edge in the ProcesorNetwork, either a PortConnection or a PropertyLink, as a
 * pair of paths, i.e. dot separated identifiers.
 */
struct IVW_CORE_API NetworkEdge : Serializable {
    NetworkEdge() = default;
    NetworkEdge(std::string src, std::string dst);

    NetworkEdge(const PropertyLink& link);
    NetworkEdge(const PortConnection& connection);

    virtual ~NetworkEdge() = default;

    PortConnection toConnection(const ProcessorNetwork& net) const;
    PropertyLink toLink(const ProcessorNetwork& net) const;

    /**
     * @brief Update the processor identifier in the srcPath with new ids from the map
     * @param map with replacement identifiers
     */
    void updateSrcProcessorID(const std::map<std::string, std::string, std::less<>>& map);
    /**
     * @brief Update the processor identifier in the dstPath with new ids from the map
     * @param map with replacement identifiers
     */
    void updateDstProcessorID(const std::map<std::string, std::string, std::less<>>& map);
    /**
     * @brief Update the processor identifiers in the srcPath and dstPath with new ids from the map
     * @param map with replacement identifiers
     */
    void updateProcessorID(const std::map<std::string, std::string, std::less<>>& map);

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    std::string srcPath;
    std::string dstPath;
};

}  // namespace inviwo
