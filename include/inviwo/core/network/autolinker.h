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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/network/processornetwork.h>

namespace inviwo {

class Processor;

/**
 * \brief Utility class to calculate and apply auto linking
 */
class IVW_CORE_API AutoLinker {
public:
    /**
     * Construct an auto link helper
     * @param network the processor network in which to add autolinks
     * @param target the processor onto which auto links should be added
     * @param source source of links. If source is not null we consider all predecessors of source
     * as input for linking, if source is null we consider all processor in the network
     */
    AutoLinker(ProcessorNetwork* network, Processor* target, Processor* source = nullptr);
    virtual ~AutoLinker() = default;

    const std::unordered_map<Property*, std::vector<Property*>>& getAutoLinkCandidates() const;

    void sortAutoLinkCandidates();
    void sortAutoLinkCandidates(dvec2 pos);

    void addLinksToClosestCandidates(bool bidirectional);

    static void addLinks(ProcessorNetwork* network, Processor* target, Processor* source = nullptr);

private:
    ProcessorNetwork* network_;
    Processor* target_;
    std::unordered_map<Property*, std::vector<Property*>> autoLinkCandiates_;
};

}  // namespace inviwo
