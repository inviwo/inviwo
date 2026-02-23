/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertyobserver.h>
#include <inviwo/core/network/processornetworkobserver.h>
#include <inviwo/core/properties/propertyownerobserver.h>

namespace inviwo {

class InviwoApplication;
class ProcessorNetwork;
class ProcessorNetworkEvaluator;
class SequenceCompositeSinkBase;
class SequenceCompositeSourceBase;
class ResizeEvent;
class Event;

/**
 * @brief A processor that contains and manages a sub network and evaluates that sub network once
 * per item in an input sequence.
 *
 * The SequenceProcessor encapsulates a group of processors (the sub network) and applies the same
 * sub network computation to every element of an input sequence. Unlike a standard composite
 * processor that evaluates its sub network exactly once per call to process(), the
 * SequenceProcessor iterates over an input sequence and evaluates the sub network for each item in
 * turn. This is useful when you want to run identical processing steps for each element in a
 * collection (e.g., frames, volumes, or any sequence of data).
 *
 * Typical workflow:
 *  - Create a sequence composite by selecting a group of processors in the network editor and
 *    choosing "create sequence".
 *  - The SequenceProcessor exposes special "super" inports and outports that bridge the super
 *    network (the enclosing network) and the sub network. SequenceCompositeSourceBase processors
 *    inside the sub network act as per-item inputs: for each item the SequenceProcessor places the
 *    current item on the super inport and the source processor forwards it into the sub network.
 *    SequenceCompositeSinkBase processors act as per-item outputs: after processing a single item
 *    the sink processors forward the result back to the super network.
 *
 * Behavior when running:
 *  - The SequenceProcessor iterates the input sequence.
 *  - For each item it provides the item to the sub network via SequenceCompositeSourceBase(s),
 *    evaluates the sub network for that single item, and collects per-item outputs from
 *    SequenceCompositeSinkBase(s) back to the super network.
 *
 * Properties in the sub network added via addSuperProperty, are cloned and exposed on the
 * SequenceProcessor. The cloned properties are synchronized with the originals using mutual
 * onChange callbacks so that state is kept consistent between the super and sub networks.
 *
 * Events are forwarded through the sub network using the super inports and outports.
 *
 * @see SequenceCompositeSourceBase
 * @see SequenceCompositeSinkBase
 */
class IVW_CORE_API SequenceProcessor : public Processor,
                                       public ProcessorNetworkObserver,
                                       public PropertyOwnerObserver,
                                       public PropertyObserver {
public:
    /**
     * Construct a SequenceProcessor. An optional workspace file can be supplied in which case it
     * is deserialized as the SequenceProcessor's sub network; otherwise the sub network is left
     * empty. Call getSubNetwork() to add processors programmatically.
     */
    SequenceProcessor(std::string_view identifier, std::string_view displayName,
                      InviwoApplication* app, const std::filesystem::path& filename = {});
    SequenceProcessor(const SequenceProcessor&) = delete;
    SequenceProcessor(SequenceProcessor&&) = delete;
    SequenceProcessor& operator=(const SequenceProcessor&) = delete;
    SequenceProcessor& operator=(SequenceProcessor&&) = delete;
    virtual ~SequenceProcessor();

    /**
     * Evaluate the sub network for each item in the input sequence.
     */
    virtual void process() override;

    /**
     * Propagate events for the current sequence item through the sub network using the sink and
     * source processors.
     */
    virtual void propagateEvent(Event* event, Outport* source) override;

    /**
     * Save the current sub network into the sequence composites folder of the user settings dir.
     * The current display name will be used as filename. Saved networks will automatically appear
     * as Processors in the processor list with the same display name.
     */
    void saveSubNetwork(const std::filesystem::path& file);

    /**
     * Get access to the sub network to add or remove processors etc
     */
    ProcessorNetwork& getSubNetwork();

    /**
     * Add a corresponding property in the SequenceProcessor for the sub property 'subProperty' in
     * the sub network. Changes in the subProperty will be reflected in the superProperty and vice
     * versa.
     */
    Property* addSuperProperty(Property* subProperty);

    /**
     * Get the super property for sub property 'subProperty' given there is one, nullptr otherwise.
     */
    Property* getSuperProperty(Property* subProperty);

    /**
     * Remove the super property for sub property 'subProperty'.
     */
    void removeSuperProperty(Property* subProperty);

    /**
     * Get the sub property for super property 'superProperty'.
     */
    Property* getSubProperty(Property* superProperty);

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;
    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    struct NetEval {
        std::unique_ptr<ProcessorNetwork> net;
        std::unique_ptr<ProcessorNetworkEvaluator> eval;
        std::vector<SequenceCompositeSinkBase*> sinks;
        std::vector<SequenceCompositeSourceBase*> sources;
    };
    void createNetworkCopies(size_t count);

    // Keeps track of the sup/super property pairs.
    struct IVW_CORE_API PropertyHandler {
        PropertyHandler(SequenceProcessor& composite, Property* subProperty);
        PropertyHandler(const PropertyHandler&) = delete;
        PropertyHandler(PropertyHandler&&) = delete;
        PropertyHandler& operator=(const PropertyHandler&) = delete;
        PropertyHandler& operator=(PropertyHandler&&) = delete;
        ~PropertyHandler();

        SequenceProcessor& comp;
        Property* subProperty;
        Property* superProperty = nullptr;
        const BaseCallBack* subCallback = nullptr;
        const BaseCallBack* superCallback = nullptr;
        bool onChangeActive = false;
        PropertyObserverDelegate superObserver;
    };

    void loadSubNetwork(const std::filesystem::path& file);

    void registerProperty(Property* subProperty);
    void unregisterProperty(Property* subProperty);

    // PropertyObserver overrides
    virtual void onSetIdentifier(Property* subProperty, const std::string& identifier) override;
    virtual void onSetDisplayName(Property* subProperty, const std::string& displayName) override;
    virtual void onSetSemantics(Property* subProperty, const PropertySemantics& semantics) override;
    virtual void onSetReadOnly(Property* subProperty, bool readonly) override;
    virtual void onSetVisible(Property* subProperty, bool visible) override;

    // ProcessorNetworkObserver overrides
    virtual void onProcessorNetworkDidAddProcessor(Processor*) override;
    virtual void onProcessorNetworkWillRemoveProcessor(Processor*) override;
    virtual void onProcessorNetworkEvaluateRequest() override;
    virtual void onProcessorBackgroundJobsChanged(Processor*, int, int) override;

    // PropertyOwnerObserver overrides
    virtual void onDidAddProperty(Property* property, size_t index) override;
    virtual void onWillRemoveProperty(Property* property, size_t index) override;

    InviwoApplication* app_;

    bool isProcessing_ = false;
    std::unordered_map<Property*, std::unique_ptr<PropertyHandler>> handlers_;

    NetEval sub_;
    std::vector<NetEval> copies_;
    std::unique_ptr<ResizeEvent> lastResize_;
};

}  // namespace inviwo
