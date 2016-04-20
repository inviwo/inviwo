/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_PROCESSORNETWORK_H
#define IVW_PROCESSORNETWORK_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorobserver.h>
#include <inviwo/core/properties/propertyownerobserver.h>
#include <inviwo/core/network/portconnection.h>
#include <inviwo/core/network/processornetworkobserver.h>
#include <inviwo/core/links/propertylink.h>
#include <inviwo/core/links/linkevaluator.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/inviwosetupinfo.h>
#include <inviwo/core/network/networklock.h>

namespace inviwo {

class InviwoApplication;

/**
 * This class manages the current processor network. It can be thought of as a container of
 * Processor instances, which Port instances are connected through PortConnection instances,
 * and which Property instances are linked through ProcessorLink instances. To manage Processors,
 * PortConnections and ProcessorLinks, add and remove methods are available for all
 * these entities. The network should be only modified by using these methods. Typically,
 * these methods are called by the NetworkEditor, which enables the user to edit
 * ProcessorNetworks.
 *
 * When the ProcessorNetwork has been changed, it is flagged as modified. This mechanism is
 * used by the ProcessorNetworkEvaluator to identify if the ProcessorNetwork needs to be
 * analyzed before processing.
 *
 * In the model view controller design pattern, the ProcessorNetwork represents the model,
 * which means that no graphical representations are generated for the added entities. Adding
 * and removing of the graphical representations is done in the NetworkEditor.
 */
class IVW_CORE_API ProcessorNetwork : public Serializable,
                                      public ProcessorNetworkObservable,
                                      public ProcessorObserver,
                                      public PropertyOwnerObserver {
public:
    using ProcessorMap = std::unordered_map<std::string, Processor*>;
    using PortConnections = std::unordered_set<PortConnection>;
    using PropertyLinks = std::unordered_set<PropertyLink>;

    ProcessorNetwork(InviwoApplication* application);
    virtual ~ProcessorNetwork();

    /**
     * Adds a Processor to the ProcessorNetwork. The identifiers of all processors in the
     * ProcessorNetwork should be unique.
     * @param[in] processor The Processor to be added.
     * @see removeProcessor(), Processor::setIdentifier()
     */
    bool addProcessor(Processor* processor);

    /**
     * Removes a Processor from the ProcessorNetwork. To ensure that the network does not end up
     * in a corrupt state, this method first removes and deletes all PortConnections and
     * ProcessorLinks, which are related to the Processor to be removed.
     * @param[in] processor The Processor to be removed.
     * @see addProcessor()
     */
    void removeProcessor(Processor* processor);

    /**
    * Removes and deletes a Processor from the ProcessorNetwork. To ensure that the network does not
    * end up in a corrupt state, this method first removes and deletes all PortConnections and
    * ProcessorLinks, which are related to the Processor to be removed.
    * @param[in] processor The Processor to be removed.
    * @see addProcessor()
    */
    void removeAndDeleteProcessor(Processor* processor);

    /**
    * Returns the Processor from the ProcessorNetwork, which has the given identifier.
    * In case no Processor with the given identifier is contained in the network, a null
    * pointer is returned.
    * @param identifier Identifier of the Processor to be accessed.
    * @see getProcessorsByType(), Processor::setIdentifier(), Processor::getIdentifier()
    */
    Processor* getProcessorByIdentifier(std::string identifier) const;

    /**
    * Returns a vector of Processors which are of type T. In case no Processors match T
    * an empty vector is returned.
    * @see getProcessorByIdentifier()
    */
    template <class T>
    std::vector<T*> getProcessorsByType() const;

    std::vector<Processor*> getProcessors() const;

    /**
    * Adds a PortConnection to the ProcessorNetwork. This involves creating the connection
    * between the two specified ports, as well as adding this connection to the ProcessorNetwork.
    * @param[in] sourcePort The outport.
    * @param[in] destPort The inport.
    * @see removeConnection()
    */
    void addConnection(Outport* sourcePort, Inport* destPort);
    void addConnection(const PortConnection& connection);


    /**
     * Removes and deletes a PortConnection from the ProcessorNetwork. This involves resolving the
     * connection between the two specified Ports, as well as removing this connection from the
     * ProcessorNetwork.
     * @param[in] sourcePort The outport.
     * @param[in] destPort The inport.
     * @see addConnection()
     */
    void removeConnection(Outport* sourcePort, Inport* destPort);
    void removeConnection(const PortConnection& connection);

    /**
    * Checks weather two port are connected
    * @param[in] sourcePort The outport.
    * @param[in] destPort The inport.
    * @return Weather the two port are connected
    * @see addConnection()
    */
    bool isConnected(Outport* sourcePort, Inport* destPort) const;
    bool isConnected(const PortConnection& connection) const;

    const std::vector<PortConnection>& getConnections() const;

    bool isPortInNetwork(Port* port) const;

    /**
     * Create and add Property Link to the network
     * Adds a link between two properties, that are owned by processor network.
     * @param[in] sourceProperty Property at which link starts
     * @param[in] destinationProperty Property at which link ends
     * @return PropertyLink* Newly added link
     */
    void addLink(Property* source, Property* destination);
    void addLink(const PropertyLink& link);

    /**
     * Remove and delete Property Link from the network
     * Removes a link between two properties, that are owned by processor network.
     * @param[in] sourceProperty Property at which link starts
     * @param[in] destinationProperty Property at which link ends
     * @return void
     */
    void removeLink(Property* source, Property* destination);
    void removeLink(const PropertyLink& link);

    /**
     * Check whether Property Link exists
     * Checks if there is a link between two properties, that are owned by processor network.
     * @param[in] sourceProperty Property at which link starts
     * @param[in] destinationProperty Property at which link ends
     * @return bool true if link exists otherwise returns false
     */
    bool isLinked(Property* source, Property* destination) const;
    bool isLinked(const PropertyLink& link) const;

    std::vector<PropertyLink> getLinks() const;
    /**
      * Is Property Link bidirectional
      * Searches for bidirectional link between start and end properties
      * In other words property that goes from end to start
      * @param[in] source Property at which link starts
      * @param[in] destination Property at which link ends
      * @return void
      */
    bool isLinkedBidirectional(Property* source, Property* destination);

    std::vector<Property*> getPropertiesLinkedTo(Property* property);
    std::vector<PropertyLink> getLinksBetweenProcessors(Processor* p1, Processor* p2);

    Property* getProperty(std::vector<std::string> path) const;
    bool isPropertyInNetwork(Property* prop) const;

    InviwoApplication* getApplication() const;

    void autoLinkProcessor(Processor* processor);
    void evaluateLinksFromProperty(Property*);

    void modified();
    void setModified(bool modified);
    bool isModified() const;
    bool isInvalidating() const;
    bool isLinking() const;

    // ProcessorObserver overrides.
    virtual void onAboutPropertyChange(Property*) override;
    virtual void onProcessorInvalidationBegin(Processor*) override;
    virtual void onProcessorInvalidationEnd(Processor*) override;
    virtual void onProcessorRequestEvaluate(Processor* p = nullptr) override;
    virtual void onProcessorIdentifierChange(Processor*) override;
    virtual void onProcessorPortRemoved(Processor*, Port* port) override;

    void lock();
    void unlock();
    bool islocked() const;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;
    bool isDeserializing() const;

    /**
    * Clears the network objects processors, port connections, property links etc.,
    */
    void clear();

    virtual void onWillRemoveProperty(Property* property, size_t index) override;

private:
    void addPropertyOwnerObservation(PropertyOwner*);
    void removePropertyOwnerObservation(PropertyOwner*);

    struct ErrorHandle {
        ErrorHandle(const InviwoSetupInfo& info, const Deserializer& d) : info_(info), d_(d) {};

        ~ErrorHandle() {
            if (!messages.empty()) {
                LogNetworkError("There were errors while loading workspace: " + d_.getFileName() +
                               "\n" + joinString(messages, "\n"));
            }
        }

        void operator()(ExceptionContext c) {
            try {
                throw;
            } catch (SerializationException& error) {
                auto key = error.getKey();
                if (key == "Processor") {
                    std::string module = info_.getModuleForProcessor(error.getType());
                    if (!module.empty()) {
                        messages.push_back(error.getMessage() + " Processor was in module: \"" +
                                           module + "\".");
                    } else {
                        messages.push_back(error.getMessage());
                    }
                } else {
                    messages.push_back(error.getMessage());
                }
            } catch (Exception& exception) {
                messages.push_back("Deserialization error: " + exception.getMessage());
            }
        }

        std::vector<std::string> messages;
        const InviwoSetupInfo& info_;
        const Deserializer& d_;
    };

    static const int processorNetworkVersion_;

    bool modified_ = true;
    unsigned int locked_ = 0;
    bool deserializing_ = false;

    InviwoApplication* application_;

    ProcessorMap processors_;
    PortConnections connections_;
    // This vector is needed to keep the connections in chronological order, since some processors
    // depends on the order of connections, ideally we would get rid of this.
    std::vector<PortConnection> connectionsVec_;
    PropertyLinks links_;

    LinkEvaluator linkEvaluator_;
    std::vector<Processor*> processorsInvalidating_;
};

template <class T>
std::vector<T*> ProcessorNetwork::getProcessorsByType() const {
    std::vector<T*> processors;
    for (ProcessorMap::const_iterator it = processors_.begin(); it != processors_.end(); ++it) {
        T* processor = dynamic_cast<T*>(it->second);
        if (processor) processors.push_back(processor);
    }
    return processors;
}

inline void ProcessorNetwork::lock() { locked_++; }
inline void ProcessorNetwork::unlock() {
    (locked_ > 0) ? locked_-- : locked_ = 0;
    if (locked_ == 0) notifyObserversProcessorNetworkUnlocked();
}
inline bool ProcessorNetwork::islocked() const { return (locked_ != 0); }

}  // namespace

#endif  // IVW_PROCESSORNETWORK_H
