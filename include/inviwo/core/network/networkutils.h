/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/stdextensions.h>

#include <functional>

namespace inviwo::util {

using OffsetCallback = std::function<ivec2(std::vector<Processor*>)>;

namespace detail {

/**
 * Helper class for Copy/Pasting a network with sub parts referring to stuff outside of the network.
 */
struct PartialProcessorNetwork : public Serializable {
    explicit PartialProcessorNetwork(ProcessorNetwork* network, OffsetCallback callback = nullptr);

    std::vector<Processor*> getAddedProcessors() const;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    ProcessorNetwork* network_;
    std::vector<Processor*> addedProcessors_;
    OffsetCallback callback_;
};

}  // namespace detail

IVW_CORE_API std::unordered_set<Processor*> getDirectPredecessors(Processor* processor);
IVW_CORE_API std::unordered_set<Processor*> getDirectSuccessors(Processor* processor);

IVW_CORE_API std::unordered_set<Processor*> getPredecessors(Processor* processor);
IVW_CORE_API std::unordered_set<Processor*> getSuccessors(Processor* processor);

enum class TraversalDirection { Up, Down };
enum class VisitPattern { Pre, Post };

struct DefaultTraversalFilter {
    bool operator()(Processor*, Inport*, Outport*) { return true; }
    bool operator()(Processor*, Outport*, Inport*) { return true; }
};

template <TraversalDirection D, VisitPattern V, typename Func,
          typename Filter = DefaultTraversalFilter>
void traverseNetwork(std::unordered_set<Processor*>& state, Processor* processor, Func f,
                     Filter connectionFilter = DefaultTraversalFilter{}) {
    if (state.count(processor) == 0) {
        state.insert(processor);

        if constexpr (V == VisitPattern::Pre) f(processor);

        if constexpr (D == TraversalDirection::Up) {
            for (auto port : processor->getInports()) {
                for (auto connectedPort : port->getConnectedOutports()) {
                    if (connectionFilter(processor, port, connectedPort)) {
                        traverseNetwork<D, V, Func>(state, connectedPort->getProcessor(), f,
                                                    connectionFilter);
                    }
                }
            }
        } else {
            for (auto port : processor->getOutports()) {
                for (auto connectedPort : port->getConnectedInports()) {
                    if (connectionFilter(processor, port, connectedPort)) {
                        traverseNetwork<D, V, Func>(state, connectedPort->getProcessor(), f,
                                                    connectionFilter);
                    }
                }
            }
        }

        if constexpr (V == VisitPattern::Post) f(processor);
    }
}

IVW_CORE_API std::vector<Processor*> topologicalSortFiltered(ProcessorNetwork* network);

IVW_CORE_API std::vector<Processor*> topologicalSort(ProcessorNetwork* network);

struct IVW_CORE_API PropertyDistanceSorter {
    PropertyDistanceSorter();
    void setTarget(vec2 pos);
    void setTarget(const Property* target);
    bool operator()(const Property* a, const Property* b);

private:
    vec2 getPosition(const Property* p);
    vec2 getPosition(const Processor* processor);

    vec2 pos_ = {0, 0};
    std::map<const Property*, vec2> cache_;
};

/**
 * Retrieve the positions of the processors in the list.
 */
IVW_CORE_API std::vector<ivec2> getPositions(const std::vector<Processor*>& processors);

/**
 * Retrieve the positions of the processors in the network.
 */
IVW_CORE_API std::vector<ivec2> getPositions(ProcessorNetwork* network);

/**
 * Retrieve the mean position of the processors in the list.
 */
IVW_CORE_API ivec2 getCenterPosition(const std::vector<Processor*>& processors);

/**
 * Retrieve the mean position of the processors in the network.
 */
IVW_CORE_API ivec2 getCenterPosition(ProcessorNetwork* network);

/**
 * Retrieve bounding box of the processors in the list.
 * The return value is pair of the min x,y and the max x,y
 */
IVW_CORE_API std::pair<ivec2, ivec2> getBoundingBox(const std::vector<Processor*>& processors);

/**
 * Retrieve bounding box of the processors in the network.
 * The return value is pair of the min x,y and the max x,y
 */
IVW_CORE_API std::pair<ivec2, ivec2> getBoundingBox(ProcessorNetwork* network);

/**
 * Offset all the positions of the processors in the list by offset
 */
IVW_CORE_API void offsetPosition(const std::vector<Processor*>& processors, ivec2 offset);

/**
 * Set the listed processors as selected or unSelected.
 */
IVW_CORE_API void setSelected(const std::vector<Processor*>& processors, bool selected);

IVW_CORE_API void serializeSelected(ProcessorNetwork* network, std::ostream& os,
                                    const std::filesystem::path& refPath);

/**
 * Append a PartialProcessorNetwork to the network
 * @param network the network to append to the current one
 * @param is a stream of a serialized PartialProcessorNetwork
 * @param refPath a possible path to the original file of the PartialProcessorNetwork, for error
 * reporting
 * @param app The inviwo application
 * @param offsetCallback  callback for determining an offset, which is applied to all added
 * processors
 * @return the appended processors.
 */
IVW_CORE_API std::vector<Processor*> appendPartialProcessorNetwork(
    ProcessorNetwork* network, std::istream& is, const std::filesystem::path& refPath,
    InviwoApplication* app, OffsetCallback offsetCallback = nullptr);

IVW_CORE_API std::vector<Processor*> appendProcessorNetwork(
    ProcessorNetwork* destinationNetwork, const std::filesystem::path& workspaceFile,
    InviwoApplication* app);

IVW_CORE_API bool addProcessorOnConnection(ProcessorNetwork* network,
                                           std::shared_ptr<Processor> processor,
                                           PortConnection connection);

IVW_CORE_API std::shared_ptr<Processor> replaceProcessor(ProcessorNetwork* network,
                                                         std::shared_ptr<Processor> newProcessor,
                                                         Processor* oldProcessor);

}  // namespace inviwo::util
