/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/processors/processorobserver.h>
#include <inviwo/core/interaction/events/eventpropagator.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processorstatus.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/util/statecoordinator.h>
#include <inviwo/core/util/dispatcher.h>

#include <memory>

namespace inviwo {

class Event;
class InteractionHandler;
class ProcessorWidget;
class ProcessorNetwork;
class NetworkVisitor;
class InviwoApplication;

/**
 * \defgroup processors Processors
 */

/**
 * \ingroup processors
 * \brief A processor generally performs operation on input data and outputs the new result.
 *
 * It can hold arbitrary number of inports and outports, as well as properties which can be used
 * to customize the processors behavior.
 *
 * A typical flow for processing Processor 1 is shown below.
 *
 * \verbatim
 *     ┌─────────────┐
 *     │             │
 *     │ Processor 2 │
 *     │ ┌─┐         │◀────────────────────────────────────────── Outport 2
 *     └─┴┬┴─────────┘
 *        └────────────────┐
 *                       ┌┬▼┬──────────┐ ◀─────────────────────── Inport 1
 *                       │└─┘          │
 *                       │ Processor 1 │
 *                       │┌─┐          │ ◀─────────────────────── Outport 1
 *                       └┴┬┴──────────┘
 *                         └────────────────┐
 *                                        ┌┬▼┬──────────┐ ◀────── Inport 0
 *                                        │└─┘          │
 *                                        │ Processor 0 │
 *                                        │┌─┐          │ ◀────── Outport 0
 *                                        └┴─┴──────────┘
 *
 *       Evaluator         Processor 1         Inport 1           Outport 2
 *           │                  │                  │                  │
 *           │                  │                  │                  │
 *           │                  │                  │                  │
 *           ├─────!isValid?────▶                  │                  │
 *           ◀────────No────────┤                  │                  │
 *           │                  │                  │                  │
 *           │                  │                  │                  │
 *           │                  │                  │                  │
 *           ├────isReady?──────▶                  │                  │
 *           │                  ├─────isValid?─────▶                  │
 *           │                  │                  ├────isValid?──────▶
 *           │                  │                  │                  ├─────┐
 *           │                  │                  │                  │  data?
 *           │                  │                  │                  │  valid?
 *           │                  │                  │                  ◀─Yes─┘
 *           │                  │                  ◀──────Yes─────────┤
 *           │                  ◀───────Yes────────┤                  │
 *           ◀──────Yes─────────┤                  │                  │
 *           │                  │                  │                  │
 *           │                  │                  │                  │
 *           │    Inv. Level    │                  │                  │
 *           ├─────────>────────▶                  │                  │
 *           │   INV.RESOURCES  │                  │                  │
 *           │                  │                  │                  │
 *           ◀───────Yes────────┤                  │                  │
 *           │                  │                  │                  │
 *           │                  │                  │                  │
 *           │                  │                  │                  │
 *           ├──Init. Resources ▶                  │                  │
 *           ◀───────Done───────┤                  │                  │
 *           │                  │                  │                  │
 *           │                  │                  │                  │
 *           │                  │                  │                  │
 *           ├────GetInports────▶                  │                  │
 *           │                  ├──callOnChange────▶                  │
 *           │                  ◀──────Done────────┤                  │
 *           ◀───────Done───────┤                  │                  │
 *           │                  │                  │                  │
 *           │                  │                  │                  │
 *           │                  │                  │                  │
 *           ├────Process───────▶                  │                  │
 *           │                  ├─────GetData──────▶                  │
 *           │                  │                  ├────GetData───────▶
 *           │                  │                  ◀──────data────────┤
 *           │                  ◀───────data───────┤                  │
 *           │                  │                  │                  │
 *           │                  │                  ▼                  ▼
 *           │                  │
 *           │                  │              Outport 1
 *           │                  │                  │
 *           │                  ├─────SetData──────▶
 *           │                  ◀───────Done───────┤
 *           ◀───────Done───────┤                  │
 *           │                  │                  ▼
 *           │                  │
 *           │                  │              Inport 1
 *           ├─────SetValid─────▶    SetChanged    │
 *           │                  ├─────(false)──────▶
 *           │                  ◀───────Done───────┤
 *           │                  │                  │
 *           │                  │                  ▼
 *           │                  │
 *           │                  │              Outport 1          Inport 0
 *           │                  │                  │                  │
 *           │                  ├───SetValid───────▶                  │
 *           │                  │                  ├─────SetValid─────▶
 *           │                  │                  │                  ├──────┐
 *           │                  │                  │                  │ SetChanged
 *           │                  │                  │                  │   (true)
 *           │                  │                  │                  ◀──────┘
 *           │                  │                  ◀──────Done────────┤
 *           │                  ◀─────Done─────────┤                  │
 *           ◀───────Done───────┤                  │                  │
 *           ▼                  ▼                  ▼                  ▼
 *
 * \endverbatim
 */
class IVW_CORE_API Processor : public PropertyOwner,
                               public MetaDataOwner,
                               public ProcessorObservable,
                               public EventPropagator,
                               public std::enable_shared_from_this<Processor> {
public:
    using NameDispatcher = Dispatcher<void(std::string_view, std::string_view)>;
    using NameDispatcherHandle = typename NameDispatcher::Handle;

    /**
     * Processor constructor, takes two optional arguments.
     * @param identifier Should only contain alpha numeric
     *                   characters, "-", "_" and " ".
     * @param displayName Name of processor, arbitrary string.
     * If the parameters are not set, the processor factory will initiate the
     * identifier and displayName with the ProcessorInfo displayName.
     */
    Processor(std::string_view identifier = "", std::string_view displayName = "");
    virtual ~Processor();

    // Should be implemented by all inheriting classes;
    virtual const ProcessorInfo getProcessorInfo() const = 0;
    std::string getClassIdentifier() const;
    std::string getCategory() const;
    CodeState getCodeState() const;
    Tags getTags() const;

    /**
     * Sets the identifier of the Processor. Processor identifiers should only contain alpha numeric
     * characters, "-", "_" and " ". If there already exist a processor with that identifier or if
     * the identifier is invalid an Exception will be thrown. By default initialized to the
     * ProcessorInfo displayName.
     * When adding the processor to a network the network will use util::findUniqueIdentifier
     * to modify the identifier if it is already used in the network.
     * @see ProcessorNetwork
     * @see util::findUniqueIdentifier
     */
    void setIdentifier(std::string_view identifier);
    virtual const std::string& getIdentifier() const override;

    /**
     * Get notified when the processor identifier changes. The callback happens after the identifier
     * is changed.
     * @param callback gets called with the new and the old identifier respectively.
     */
    NameDispatcherHandle onIdentifierChange(
        std::function<void(std::string_view, std::string_view)> callback);

    /**
     * Name of processor, arbitrary string. By default initialized to the ProcessorInfo displayName.
     * This name will be shown on various graphical representations.
     */
    void setDisplayName(std::string_view displayName);
    const std::string& getDisplayName() const;

    /**
     * Get notified when the processor display name changes. The callback happens after the display
     * name is changed.
     * @param callback gets called with the new and the old display name respectively.
     */
    NameDispatcherHandle onDisplayNameChange(
        std::function<void(std::string_view, std::string_view)> callback);

    virtual void setProcessorWidget(std::unique_ptr<ProcessorWidget> processorWidget);
    ProcessorWidget* getProcessorWidget() const;
    bool hasProcessorWidget() const;

    virtual void setNetwork(ProcessorNetwork* network);
    ProcessorNetwork* getNetwork() const;

    /**
     * InitializeResources is called whenever a property with InvalidationLevel::InvalidResources
     * is changes.
     */
    virtual void initializeResources() {}

    Port* getPort(std::string_view identifier) const;
    Inport* getInport(std::string_view identifier) const;
    Outport* getOutport(std::string_view identifier) const;

    const std::vector<Inport*>& getInports() const;
    const std::vector<Outport*>& getOutports() const;
    /**
     * Concept for event propagation. Currently only used for
     * ResizeEvents, which only propagate from outports to inports in the same port group
     */
    const std::string& getPortGroup(Port* port) const;
    std::vector<std::string> getPortGroups() const;
    const std::vector<Port*>& getPortsInGroup(std::string_view portGroup) const;
    const std::vector<Port*>& getPortsInSameGroup(Port* port) const;

    bool allInportsConnected() const;

    /**
     * The default function for checking whether all inports are ready. Will return true if all non
     * optional inports are ready.
     */
    bool allInportsAreReady() const;

    /**
     * Returns whether the processor is a source. I.e. whether it brings data into the network.
     * By default a processor is a source if it has no inports. This behavior can be customized by
     * setting the isSource_ update functor.
     * @see StateCoordinator
     */
    bool isSource() const;

    /**
     * Returns whether the processor is a sink. I.e. whether it pulls data from the network.
     * By default a processor is a sink if it has no outports. This behavior can be customized by
     * setting the isSink_ update functor. For a processor to be evaluated there have to be a sink
     * among its descendants.
     * @see StateCoordinator
     */
    bool isSink() const;

    /**
     * Returns whether the processor is ready to be processed. By default this will call
     * allInportsAreReady. This behavior can be customized by setting the isReady_ update functor.
     * @see StateCoordinator
     * @see allInportsAreReady
     */
    bool isReady() const;

    /**
     * Returns whether the processor is ready to be processed. By default this will call
     * allInportsAreReady. This behavior can be customized by setting the isReady_ update functor.
     * @see StateCoordinator
     * @see allInportsAreReady
     */
    const ProcessorStatus& status() const;

    /**
     * Deriving classes should override this function to do the main work of the processor.
     * This function is called by the ProcessorNetworkEvaluator when the network is evaluated and
     * the processor is invalid, i.e. some of its inports or properties has been modified, and is
     * ready i.e. all non optional inports have valid data.
     * @see ProcessorNetworkEvaluator
     * @see Inport
     * @see Property
     */
    virtual void process() {}

    /**
     * This function is called by the ProcessorNetworkEvaluator when the network is evaluated and
     * the processor is neither ready or valid.
     * @see ProcessorNetworkEvaluator
     */
    virtual void doIfNotReady() {}

    /**
     * Called by the network after Processor::process has been called.
     * This will set the following to valid
     *   * The processor
     *   * All properties
     *   * All outports and their connected inports.
     * It will also set its inports "changed" to false.
     */
    virtual void setValid() override;

    /**
     * Triggers invalidation.
     * Perform only full reimplementation of this function, meaning never call
     * Processor::invalidate()
     * in your reimplemented invalidation function.
     * The general scheme is that the processor will invalidate is self and it's outports
     * the outports will in turn invalidate their connected inports, which will invalidate their
     * processors. Hence all processors that depend on this one in the network will be invalidated.
     */
    virtual void invalidate(InvalidationLevel invalidationLevel,
                            Property* modifiedProperty = nullptr) override;

    /**
     * Adds the interaction handler such that it receives events propagated
     * to the processor. Will not add the interaction handler if it has been added before.
     * @note The processor will not take ownership of the interaction handler.
     * @see InteractionHandler
     * @param interactionHandler Interaction handler to be added.
     */
    void addInteractionHandler(InteractionHandler* interactionHandler);
    /**
     * Remove the interaction handler such that events are not propagated.
     * The interaction handler will not be deleted by the processor.
     * @param interactionHandler Interaction handler to be removed.
     */
    void removeInteractionHandler(InteractionHandler* interactionHandler);
    bool hasInteractionHandler() const;
    const std::vector<InteractionHandler*>& getInteractionHandlers() const;

    virtual void invokeEvent(Event* event) override;

    // Overridden from EventPropagator.
    virtual void propagateEvent(Event* event, Outport* source) override;

    // Override from the property owner
    virtual Processor* getProcessor() override { return this; }
    virtual const Processor* getProcessor() const override { return this; }
    virtual const PropertyOwner* getOwner() const override { return nullptr; }
    virtual PropertyOwner* getOwner() override { return nullptr; };

    virtual InviwoApplication* getInviwoApplication() override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    /**
     * Add port to processor and pass ownership to processor.
     * @note Port group is a concept for event propagation. Currently only used for
     * ResizeEvents, which only propagate from outports to inports in the same port group
     * @param port to add
     * @param portGroup name of group to propagate events through (defaults to "default")
     */
    template <typename T, typename std::enable_if_t<std::is_base_of<Inport, T>::value, int> = 0>
    T& addPort(std::unique_ptr<T> port, std::string_view portGroup = "default");

    /**
     * Add port to processor and pass ownership to processor.
     * @note Port group is a concept for event propagation. Currently only used for
     * ResizeEvents, which only propagate from outports to inports in the same port group
     * @param port to add
     * @param portGroup name of group to propagate events through (defaults to "default")
     */
    template <typename T, typename std::enable_if_t<std::is_base_of<Outport, T>::value, int> = 0>
    T& addPort(std::unique_ptr<T> port, std::string_view portGroup = "default");

    /**
     * Add port to processor.
     * @note Port group is a concept for event propagation. Currently only used for
     * ResizeEvents, which only propagate from outports to inports in the same port group
     * @param port to add
     * @param portGroup name of group to propagate events through (defaults to "default")
     */
    template <typename T>
    T& addPort(T& port, std::string_view portGroup = "default");

    /**
     * Add ports to processor.
     * @note Calling this method will add all ports to the default port group. If you wish to
     * add a port to a certain port group you will need to call addPort(myPort, "myPortGroup")
     * explicitly instead.
     * @param ports to add
     */
    template <typename... Ts>
    void addPorts(Ts&... ports) {
        (addPort(ports), ...);
    }

    Port* removePort(std::string_view identifier);
    Inport* removePort(Inport* port);
    Outport* removePort(Outport* port);

    /**
     * Return true if ProcessorNetworkEvaluator should evaluate the connection during
     * ProcessorNetwork traversal. An inactive connection will propagate invalidations and events
     * but will not be processed. Useful if the Processor has states in which it does not use an
     * inport.
     * @param inport This processor's inport
     * @param outport Another processor's outport
     * @see InputSelector for an example
     */
    virtual bool isConnectionActive([[maybe_unused]] Inport* inport,
                                    [[maybe_unused]] Outport* outport) const {
        return true;
    }

    /**
     * @brief Accept a NetworkVisitor, the visitor will visit this and then each Property of the
     * Processor in an undefined order. The Visitor will then visit each Properties's properties and
     * so on.
     */
    virtual void accept(NetworkVisitor& visitor);

protected:
    std::unique_ptr<ProcessorWidget> processorWidget_;
    StateCoordinator<ProcessorStatus> isReady_;
    static std::function<ProcessorStatus()> getDefaultIsReadyUpdater(Processor*);
    StateCoordinator<bool> isSink_;
    StateCoordinator<bool> isSource_;
    /**
     * Overwrites current port group, if any.
     * @note Port group will be overwritten by addPort.
     * @see addPort
     */
    void addPortToGroup(Port* port, std::string_view portGroup);
    /**
     * Removes port from its group, even if it is the default one.
     * @see addPort
     */
    void removePortFromGroups(Port* port);

private:
    void addPortInternal(Inport* port, std::string_view portGroup);
    void addPortInternal(Outport* port, std::string_view portGroup);

    std::string identifier_;
    std::string displayName_;
    std::vector<Inport*> inports_;
    std::vector<Outport*> outports_;
    std::vector<std::unique_ptr<Inport>> ownedInports_;
    std::vector<std::unique_ptr<Outport>> ownedOutports_;
    std::vector<InteractionHandler*> interactionHandlers_;

    std::unordered_map<std::string, std::vector<Port*>> groupPorts_;
    std::unordered_map<Port*, std::string> portGroups_;

    ProcessorNetwork* network_;

    NameDispatcher identifierDispatcher_;
    NameDispatcher displayNameDispatcher_;
};

inline ProcessorNetwork* Processor::getNetwork() const { return network_; }

template <typename T, typename std::enable_if_t<std::is_base_of<Inport, T>::value, int>>
T& Processor::addPort(std::unique_ptr<T> port, std::string_view portGroup) {
    T& ret = *port;
    addPortInternal(port.get(), portGroup);
    ownedInports_.push_back(std::move(port));
    return ret;
}

template <typename T, typename std::enable_if_t<std::is_base_of<Outport, T>::value, int>>
T& Processor::addPort(std::unique_ptr<T> port, std::string_view portGroup) {
    T& ret = *port;
    addPortInternal(port.get(), portGroup);
    ownedOutports_.push_back(std::move(port));
    return ret;
}

template <typename T>
T& Processor::addPort(T& port, std::string_view portGroup) {
    static_assert(std::is_base_of<Inport, T>::value || std::is_base_of<Outport, T>::value,
                  "T must be an Inport or Outport");
    addPortInternal(&port, portGroup);
    return port;
}

}  // namespace inviwo
