/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#ifndef IVW_COMPOSITEPROCESSOR_H
#define IVW_COMPOSITEPROCESSOR_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertyobserver.h>
#include <inviwo/core/network/processornetworkobserver.h>
#include <inviwo/core/properties/propertyownerobserver.h>

namespace inviwo {

class InviwoApplication;
class ProcessorNetwork;
class ProcessorNetworkEvaluator;
class SinkProcessorBase;
class SourceProcessorBase;

/**
 * \class CompositeProcessor
 * A processor having its own processor network, called sub network.
 * We call the network of the CompositeProcessor the super network from the context of the sub
 * network. The CompositeProcessor will evaluate its sub network when it itself gets processed. It
 * will look into the sub network for any SinkProcessors, which acts as data inputs in the sub
 * network, and ask for its super inport and add it to itself. Then it will look for all
 * SinkProseccors, which acts as data outputs, and ask for its super outport and add it to it self.
 * When the SourceProcessor gets evaluated in the sub network it will take the data from its super
 * inport and put in its outport, moving the data from the super network to the sub network.
 * At the end of the sub network evaluation the SinkProcessors will be evaluated and take the data
 * on its inport and put on its super outport, moving the data from the sub network into the super
 * network.
 *
 * Properties in the sub network that are marked with application usage mode, or manually added,
 * will be cloned and added to the composite processor with mutual onChange callbacks to keep them
 * in sync.
 *
 * Events are propagated through the sub network using the super inport and outports in the Source
 * and Sink Processors.
 *
 * @see SourceProcessor
 * @see SinkProcessor
 */
class IVW_CORE_API CompositeProcessor : public Processor,
                                        public ProcessorNetworkObserver,
                                        public PropertyOwnerObserver,
                                        public PropertyObserver {
public:
    /**
     * Construct a CompositeProcessor, an optional workspace file can be supplied in which case it
     * is deserialized as the CompositeProcessors sub network. otherwise the network is left empty.
     * getSubNetwork can then be use to add processors etc.
     */
    CompositeProcessor(const std::string& identifier, const std::string& displayName,
                       InviwoApplication* app, const std::string& filename = "");
    virtual ~CompositeProcessor();

    /**
     * Evaluates the sub network
     */
    virtual void process() override;

    /**
     * Propagates events through the sub network using the sink and source processors
     */
    virtual void propagateEvent(Event* event, Outport* source) override;

    /**
     * Save the current network to the user settings dir.
     * Processors will automatically show up in the processor list. The current displayName will be
     * used to name the new sub network.
     */
    void saveSubNetwork(const std::string& file);

    /**
     * Get access to the sub network to add or remove processors etc
     */
    ProcessorNetwork& getSubNetwork();

    /**
     * Add a corresponding property in the CompositeProcessor for the sub property subProperty in
     * the sub network. Changes in the subProperty will be reflected in the superProperty and vice
     * versa.
     */
    Property* addSuperProperty(Property* subProperty);

    /**
     * Get the super property for sub property subProperty given there is one, nullptr otherwise.
     */
    Property* getSuperProperty(Property* subProperty);

    /**
     * Remove the super property for sub property subProperty.
     */
    void removeSuperProperty(Property* subProperty);

    /**
     * Get the sub property for super property superProperty
     */
    Property* getSubProperty(Property* superProperty);

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    // Keeps track of the sup/super property pairs.
    struct PropertyHandler {
        PropertyHandler(CompositeProcessor& composite, Property* subProperty);
        ~PropertyHandler();

        CompositeProcessor& comp;
        Property* subProperty;
        Property* superProperty = nullptr;
        const BaseCallBack* subCallback = nullptr;
        const BaseCallBack* superCallback = nullptr;
        bool onChangeActive = false;
    };

    void loadSubNetwork(const std::string& file);

    void registerProperty(Property* prop);
    void unregisterProperty(Property* prop);

    // PropertyObserver overrides
    virtual void onSetIdentifier(Property* property, const std::string& identifier) override;
    virtual void onSetDisplayName(Property* property, const std::string& displayName) override;
    virtual void onSetSemantics(Property* property, const PropertySemantics& semantics) override;
    virtual void onSetReadOnly(Property* property, bool readonly) override;
    virtual void onSetVisible(Property* property, bool visible) override;
    virtual void onSetUsageMode(Property* property, UsageMode usageMode) override;

    // ProcessorNetworkObserver overrides
    virtual void onProcessorNetworkDidAddProcessor(Processor*) override;
    virtual void onProcessorNetworkWillRemoveProcessor(Processor*) override;
    virtual void onProcessorNetworkEvaluateRequest() override;

    // PropertyOwnerObserver overrides
    virtual void onDidAddProperty(Property* property, size_t index) override;
    virtual void onWillRemoveProperty(Property* property, size_t index) override;

    InviwoApplication* app_;

    bool isProcessing_ = false;
    std::unordered_map<Property*, std::unique_ptr<PropertyHandler>> handlers_;
    std::vector<SinkProcessorBase*> sinks_;
    std::vector<SourceProcessorBase*> sources_;
    std::unique_ptr<ProcessorNetwork> network_;
    std::unique_ptr<ProcessorNetworkEvaluator> evaluator_;
};

}  // namespace inviwo

#endif  // IVW_COMPOSITEPROCESSOR_H
