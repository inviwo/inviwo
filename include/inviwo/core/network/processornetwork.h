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
#include <inviwo/core/network/portconnection.h>
#include <inviwo/core/network/processornetworkobserver.h>
#include <inviwo/core/links/propertylink.h>
#include <inviwo/core/links/linkevaluator.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <inviwo/core/util/inviwosetupinfo.h>

namespace inviwo {

// A Pair of processors independent of order, ProcessorPair(p1,p2) is equal to ProcessorPair(p2,p1)
class IVW_CORE_API ProcessorPair {
public:
    ProcessorPair(Processor* p1, Processor* p2);
    friend IVW_CORE_API bool operator==(const ProcessorPair& p1, const ProcessorPair& p2);
    friend IVW_CORE_API bool operator<(const ProcessorPair& p1, const ProcessorPair& p2);

    const Processor* processor1_;
    const Processor* processor2_;
};
IVW_CORE_API bool operator==(const ProcessorPair& p1, const ProcessorPair& p2);
IVW_CORE_API bool operator<(const ProcessorPair& p1, const ProcessorPair& p2);


/**
 * This class manages the current processor network. It can be thought of as a container of
 * Processor instances, which Port instances are connected through PortConnection instances,
 * and which Property instances are linked through ProcessorLink instances. To manage Processors,
 * PortConnections and ProcessorLinks, add and remove methods are available for all
 * these entities. The network should be only modified by using these methods. Typically,
 * these methods are called by the NetworkEditor, which enables the user to edit
 * ProcessorNetworks.
 *
 * When the PorcessorNetwork has been changed, it is flagged as modified. This mechanism is
 * used by the ProcessorNetworkEvaluator to identify if the ProcessorNetwork needs to be
 * analyzed before processing.
 *
 * In the model view controller design pattern, the ProcessorNetwork represents the model,
 * which means that no graphical representations are generated for the added entities. Adding
 * and removing of the graphical representations is done in the NetworkEditor.
 */
class IVW_CORE_API ProcessorNetwork : public IvwSerializable,
                                      public ProcessorNetworkObservable,
                                      public ProcessorObserver {
public:
    typedef std::map<std::string, Processor*> ProcessorMap;
    typedef std::vector<Processor*> ProcessorVector;

    typedef std::map<std::pair<Outport*, Inport*>, PortConnection*> PortConnectionMap;
    typedef std::vector<PortConnection*> PortConnectionVector;

    typedef std::map<std::pair<Property*, Property*>, PropertyLink*> PropertyLinkMap;
    typedef std::vector<PropertyLink*> PropertyLinkVector;
    typedef std::map<ProcessorPair, PropertyLinkVector> ProcessorLinkMap;

    ProcessorNetwork();
    virtual ~ProcessorNetwork();

    /**
     * Adds a Processor to the ProcessorNetwork. The identifiers of all processors in the
     * ProcessorNetwork should be unique.
     *
     * @param[in] processor The Processor to be added.
     * @see removeProcessor(), Processor::setIdentifier()
     */
    bool addProcessor(Processor* processor);

    /**
     * Removes a Processor from the ProcessorNetwork. To ensure that the network does not end up
     * in a corrupt state, this method first removes and deletes all PortConnections and ProcessorLinks, which
     * are related to the Processor to be removed.
     *
     * @param[in] processor The Processor to be removed.
     * @see addProcessor()
     */
    void removeProcessor(Processor* processor);

    /**
    * Removes and deletes a Processor from the ProcessorNetwork. To ensure that the network does not end up
    * in a corrupt state, this method first removes and deletes all PortConnections and ProcessorLinks, which
    * are related to the Processor to be removed.
    *
    * @param[in] processor The Processor to be removed.
    * @see addProcessor()
    */
    void removeAndDeleteProcessor(Processor* processor);

    /**
    * Returns the Processor from the ProcessorNetwork, which has the given identifier.
    * In case no Processor with the given identifier is contained in the network, a null
    * pointer is returned.
    *
    * @param identifier Identifier of the Processor to be accessed.
    * @see getProcessorsByType(), Processor::setIdentifier(), Processor::getIdentifier()
    */
    Processor* getProcessorByIdentifier(std::string identifier) const;

    /**
    * Returns a vector of Processors which are of type T. In case no Processors match T
    * an empty vector is returned.
    *
    * @see getProcessorByIdentifier()
    */
    template<class T> std::vector<T*> getProcessorsByType() const;

    /**
    * Returns a vector of all Processors.
    *
    * @return A vector of Processors
    */
    std::vector<Processor*> getProcessors() const;


    /**
    * Adds a PortConnection to the ProcessorNetwork. This involves creating the connection
    * between the two specified ports, as well as adding this connection to the ProcessorNetwork.
    *
    * @param[in] sourcePort The outport.
    * @param[in] destPort The inport.
    * @return The newly created connection. nullptr if a connection could not be made.
    * @see removeConnection()
    */
    PortConnection* addConnection(Outport* sourcePort, Inport* destPort);

    /**
     * Removes and deletes a PortConnection from the ProcessorNetwork. This involves resolving the connection
     * between the two specified Ports, as well as removing this connection from the
     * ProcessorNetwork.
     *
     * @param[in] sourcePort The outport.
     * @param[in] destPort The inport.
     * @see addConnection()
     */
    void removeConnection(Outport* sourcePort, Inport* destPort);

    /**
    * Checks weather two port are connected
    *
    * @param[in] sourcePort The outport.
    * @param[in] destPort The inport.
    * @return Weather the two port are connected
    * @see addConnection()
    */
    bool isConnected(Outport* sourcePort, Inport* destPort);

    /**
    * Get a connection between two ports
    *
    * @param[in] sourcePort The outport.
    * @param[in] destPort The inport.
    * @return The PortConnection between the ports or nullptr if there is none.
    * @see addConnection()
    */
    PortConnection* getConnection(Outport* sourcePort, Inport* destPort);

    /**
    * Returns a vector of all Connections.
    *
    * @return A vector of Connections
    */
    PortConnectionVector getConnections() const;

    /**
     * Create and add Property Link to the network
     *
     * Adds a link between two properties, that are owned by processor network.
     * 
     * @param[in] sourceProperty Property at which link starts
     * @param[in] destinationProperty Property at which link ends
     * @return PropertyLink* Newly added link
     */
    PropertyLink* addLink(Property* sourceProperty, Property* destinationProperty);
    /** 
     * Remove and delete Property Link from the network
     *
     * Removes a link between two properties, that are owned by processor network.
     * 
     * @param[in] sourceProperty Property at which link starts
     * @param[in] destinationProperty Property at which link ends
     * @return void 
     */
    void removeLink(Property* sourceProperty, Property* destinationProperty);
    /** 
     * Check whether Property Link exists
     *
     * Checks if there is a link between two properties, that are owned by processor network.
     * 
     * @param[in] sourceProperty Property at which link starts
     * @param[in] destinationProperty Property at which link ends
     * @return bool true if link exists otherwise returns false
     */
    bool isLinked(Property* sourceProperty, Property* destinationProperty);

    /** 
     * Find Property Link
     *
     * Search and return link between two properties, that are owned by processor network.
     * 
     * @param[in] sourceProperty Property at which link starts
     * @param[in] destinationProperty Property at which link ends
     * @return PropertyLink* returns pointer to link if it exists otherwise returns nullptr
     */
    PropertyLink* getLink(Property* sourceProperty, Property* destinationProperty) const;
    /** 
     * Return all Property Links
     * 
     * @return std::vector<PropertyLink*> List of all property links owned by processor network
     */
    PropertyLinkVector getLinks() const;
   /**
     * Get bidirectional Property Link
     *
     * Searches for bidirectional link between start and end properties
     * In other words property that goes from end to start
     * 
     * @param[in] sourceProperty Property at which link starts
     * @param[in] destinationProperty Property at which link ends
     * @return void
     */
    bool isLinkedBidirectional(Property* sourceProperty, Property* destinationProperty);

    PropertyLinkVector getLinksBetweenProcessors(Processor* p1, Processor* p2);
    /** 
     * Properties that are linked to the given property where the given property is a source property
     *
     * @param property given property
     * @return std::vector<Property*> List of all properties that are affected by given property
     */
    std::vector<Property*> getLinkedProperties(Property* property);
    std::vector<PropertyLink>& getTriggerdLinksForProperty(Property* property);


    Property* getProperty(std::vector<std::string> path) const;

    void autoLinkProcessor(Processor* processor);

    void modified();
    void setModified(bool modified);
    bool isModified() const;

    bool isLinking() const;

    bool isInvalidating() const;
    
    // ProcessorObserver overrides.
    virtual void onAboutPropertyChange(Property*);
    virtual void onProcessorInvalidationBegin(Processor*);
    virtual void onProcessorInvalidationEnd(Processor*);
    virtual void onProcessorRequestEvaluate(Processor* p = nullptr);
    virtual void onProcessorIdentifierChange(Processor*);
    
    Processor* getInvalidationInitiator();
    inline void lock() { locked_++; }
    inline void unlock() { 
        (locked_>0) ? locked_-- : locked_ = 0;
        if (locked_==0) notifyObserversProcessorNetworkUnlocked(); 
    }
    inline bool islocked() const { return (locked_!=0); }

    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d) throw (Exception);

    bool isDeserializing()const;
    void updatePropertyLinkCaches();
    void evaluatePropertyLinks(Property*);

    /**
    * Clears the network objects processors, port connections, property links etc.,
    * This function clears only the core objects and mainly used to abort any
    * further operation.
    */
    void clear();
private:
    struct ErrorHandle {
        ErrorHandle(const InviwoSetupInfo& info) : info_(info) {};
    
        void handleProcessorError(SerializationException& error) {
            std::string module = info_.getModuleForProcessor(error.getType());
            if (!module.empty()) {
                messages.push_back(error.getMessage() + " Processor was in module: \"" + module + "\"");
            } else {
                messages.push_back(error.getMessage());
            }
        }
        void handleConnectionError(SerializationException& error) {
            messages.push_back(error.getMessage());
        }
        void handleLinkError(SerializationException& error) {
            messages.push_back(error.getMessage());
        }
  
        std::vector<std::string> messages;
        const InviwoSetupInfo& info_;
    };

    class PropertyLinkContainsTest {
    public:
        PropertyLinkContainsTest(Property* p) : p_(p) {}
        bool operator()(const PropertyLink& link) {
            return link.getSourceProperty() == p_ || link.getDestinationProperty() == p_;
        }

    private:
        Property* p_;
    };

    //Property Linking support
    void performLinkingOnPropertyChange(Property* modifiedProperty);
    void addToPrimaryCache(PropertyLink* propertyLink);
    void removeFromPrimaryCache(PropertyLink* propertyLink);
    std::vector<PropertyLink>& addToSecondaryCache(Property* property);

    void secondaryCacheHelper(std::vector<PropertyLink>& links,  Property* src, Property* dst);

    void clearSecondaryCache();
    
    std::vector<Property*> getPropertiesRecursive(PropertyOwner* owner);

    static const int processorNetworkVersion_;

    // The primary link cache is a map with all source properties and a vector of properties that
    // they link directly to
    std::map<Property*, std::vector<Property*> > propertyLinkPrimaryCache_;
    // The secondary link cache is a map with all source properties and a vector of ALL the
    // properties that they link to. Directly or indirectly.
    std::map<Property*, std::vector<PropertyLink> > propertyLinkSecondaryCache_;
    // A cache of all links between two processors.
    ProcessorLinkMap processorLinksCache_;

    bool modified_;
    unsigned int locked_;
    ProcessorMap processors_;
    PortConnectionMap portConnectionsMap_;
    PortConnectionVector portConnectionsVec_;
    PropertyLinkMap propertyLinks_;

    bool deserializing_;
    bool invalidating_;
    ProcessorVector processorsInvalidating_;
    LinkEvaluator* linkEvaluator_;

    bool linking_;
    
    class NetworkConverter : public VersionConverter {
    public:
        typedef void (NetworkConverter::*updateType)(TxElement*);
        NetworkConverter(int from);
        virtual bool convert(TxElement* root);
        int from_;
    private:
        void updateProcessorType(TxElement* node);
        void updateMetaDataTree(TxElement* node);
        void updatePropertType(TxElement* node);
        void updateMetaDataType(TxElement* node);
        void updateMetaDataKeys(TxElement* node);
        void updateShadingMode(TxElement* node);
        void updateCameraToComposite(TxElement* node);
        void updateDimensionTag(TxElement* node);
        void updatePropertyLinks(TxElement* node);
        void updatePortsInProcessors(TxElement* node);

        void traverseNodes(TxElement* node, updateType update);
    };
};

template<class T>
std::vector<T*> ProcessorNetwork::getProcessorsByType() const {
    std::vector<T*> processors;
    for (ProcessorMap::const_iterator it = processors_.begin(); it != processors_.end(); ++it) {
        T* processor = dynamic_cast<T*>(it->second);
        if (processor) processors.push_back(processor);
    }
    return processors;
}

} // namespace

#endif // IVW_PROCESSORNETWORK_H
