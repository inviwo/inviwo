/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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
class CompositeSinkBase;
class CompositeSourceBase;

/**
 * \class CompositeProcessor
 * \brief A processor containing a network of processors, i.e. it will act as a sub network within a
 * processor network. A CompositeProcessor can be used to reduce cluttering in the network. Also
 * makes it easy to reuse groups of processors inside of a network, and across network since they
 * can be saved in the processor list. A CompositeProcessor is usually created by selecting a group
 * of processors that are closely related in the network editor and then clicking "create
 * composite" in the context menu.  Use InviwoModule::registerCompositeProcessor to register a saved
 * composite in your module.
 *
 * The network inside of the CompositeProcessors is called the sub network and the network of the
 * CompositeProcessor the super network. The CompositeProcessor will only evaluate its sub network
 * when its process function is called, and otherwise keep it locked.
 *
 * <b>How it works</b>
 * The CompositeProcessor will observe its sub network and when a processor gets added to the sub
 * network the CompositeProcessor will check if it's a CompositeSource or a CompositeSink. In the
 * case it's a CompositeSource, which acts as data inputs in the sub network, in will get the
 * special "super" inport and add it to it self. If it's a CompositeSink, which acts as data
 * outputs, it will get the "super" outport and add it to it self.
 *
 * When the CompositeSource gets evaluated in the sub network it will take the data from its super
 * inport and put in its outport, moving the data from the super network to the sub network. At the
 * end of the sub network evaluation the SinkProcessors will be evaluated and take the data on its
 * inport and put on its super outport, moving the data from the sub network into the super network.
 *
 * Properties in the sub network that are marked with application usage mode, or added by calling
 * addSuperProperty, will be cloned and added to the composite processor with mutual onChange
 * callbacks to keep them in sync, exposing the sub property's state to the super network.
 *
 * Events are propagated through the sub network using the super inport and outports in the Source
 * and Sink Processors.
 *
 * <b>Design considerations</b>
 * Many designs for composite processors were considered, including implementing it as a pure GUI
 * feature having all the processors in the same network. The current design of completely
 * encapsulating the sub network was chosen since it minimizes the amount of logic in the GUI. Hence
 * keeping the simple mapping from processor network to GUI. It also completely hides the sub
 * network from the super network making it possible to compose sub network in several layers out of
 * the box.
 *
 * @see CompositeSource
 * @see CompositeSink
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
     * Save the current network into the composites folder of the user settings dir. The current
     * display name will be used as filename. Saved networks will automatically appear as Processors
     * in the processor list, with the same display name.
     */
    void saveSubNetwork(const std::string& file);

    /**
     * Get access to the sub network to add or remove processors etc
     */
    ProcessorNetwork& getSubNetwork();

    /**
     * Add a corresponding property in the CompositeProcessor for the sub property 'subProperty' in
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
    std::vector<CompositeSinkBase*> sinks_;
    std::vector<CompositeSourceBase*> sources_;
    std::unique_ptr<ProcessorNetwork> subNetwork_;
    std::unique_ptr<ProcessorNetworkEvaluator> evaluator_;
};

}  // namespace inviwo

#endif  // IVW_COMPOSITEPROCESSOR_H
