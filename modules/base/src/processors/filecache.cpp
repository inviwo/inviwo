/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <modules/base/processors/filecache.h>

#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/network/portconnection.h>
#include <inviwo/core/links/propertylink.h>

#include <unordered_set>
#include <memory_resource>

namespace inviwo::detail {

std::string cacheState(Processor* processor, ProcessorNetwork& net, std::pmr::string& xml) {
    std::pmr::monotonic_buffer_resource mbr{1024 * 32};

    std::pmr::vector<Processor*> processors(&mbr);
    std::pmr::unordered_set<Processor*> state(&mbr);
    util::traverseNetwork<util::TraversalDirection::Up, util::VisitPattern::Post>(
        state, processor, [&processors](Processor* p) { processors.push_back(p); });

    // ensure a consistent order
    std::ranges::sort(processors, std::less<>{},
                      [](const Processor* p) { return p->getIdentifier(); });

    Serializer s{"", SerializeConstants::InviwoWorkspace, &mbr};
    s.setWorkspaceSaveMode(WorkspaceSaveMode::Undo);  // don't same any "metadata"
    const auto ns = s.switchToNewNode("ProcessorNetwork");
    s.serialize("ProcessorNetworkVersion", ProcessorNetwork::processorNetworkVersion());

    s.serializeRange("Processors", processors, [](Serializer& nested, const Processor* item) {
        nested.serialize("Processor", *item);
    });

    s.serializeRange(
        "Connections",
        net.connectionRange() | std::views::filter([&](const PortConnection& connection) {
            return util::contains(processors, connection.getInport()->getProcessor()) &&
                   util::contains(processors, connection.getOutport()->getProcessor());
        }),
        [](Serializer& nested, const PortConnection& connection) {
            const auto nodeSwitch = nested.switchToNewNode("Connection");
            connection.getOutport()->getPath(nested.addAttribute("src"));
            connection.getInport()->getPath(nested.addAttribute("dst"));
        });

    s.serializeRange(
        "PropertyLinks",
        net.linkRange() | std::views::filter([&](const PropertyLink& link) {
            return util::contains(
                       processors,
                       link.getDestination()->getOwner()->getProcessor()->getProcessor()) &&
                   util::contains(processors, link.getSource()->getOwner()->getProcessor());
        }),
        [](Serializer& nested, const PropertyLink& link) {
            const auto nodeSwitch = nested.switchToNewNode("PropertyLink");
            link.getSource()->getPath(nested.addAttribute("src"));
            link.getDestination()->getPath(nested.addAttribute("dst"));
        });

    xml.clear();
    s.write(xml);

    return {fmt::format("{}", std::hash<std::string_view>{}(xml))};
}

}  // namespace inviwo::detail
