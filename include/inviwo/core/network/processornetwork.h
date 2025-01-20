/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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
#include <inviwo/core/processors/processorobserver.h>
#include <inviwo/core/properties/propertyownerobserver.h>
#include <inviwo/core/network/portconnection.h>
#include <inviwo/core/network/processornetworkobserver.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/links/propertylink.h>
#include <inviwo/core/links/linkevaluator.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/iterrange.h>
#include <inviwo/core/util/transformiterator.h>

#include <string_view>
#include <memory>

namespace inviwo {

class InviwoApplication;
class NetworkVisitor;

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
                                      public PropertyOwnerObserver,
                                      public ProcessorMetaDataObserver {
public:
    explicit ProcessorNetwork(InviwoApplication* application);
    ProcessorNetwork(const ProcessorNetwork&) = delete;
    ProcessorNetwork(ProcessorNetwork&&) = delete;
    ProcessorNetwork& operator=(const ProcessorNetwork&) = delete;
    ProcessorNetwork& operator=(ProcessorNetwork&&) = delete;
    virtual ~ProcessorNetwork();

    /**
     * @brief Adds a Processor to the ProcessorNetwork.
     *
     * The identifiers of all processors in the ProcessorNetwork should be unique, if the identifier
     * is not unique, a replacement identifier will be set using util::findUniqueIdentifier. The
     * processor's network will be set to this, and any registered widget will be consructed and
     * added to the processor.
     *
     * @note A processor should only ever be added to one network at a time
     * @param[in] processor The Processor to be added.
     * @returns A pointer to the added processor
     * @see removeProcessor(), Processor::setIdentifier()
     */
    Processor* addProcessor(std::shared_ptr<Processor> processor);

    /**
     * @brief Adds a Processor to the ProcessorNetwork.
     * s
     * @see ProcessorNetwork::addProcessor(std::shared_ptr<Processor>)
     * @param[in] processor The Processor to be added.
     * @returns A pointer to the added processor of type T
     */
    template <typename T, class = typename std::enable_if_t<std::is_base_of_v<Processor, T>, void>>
    T* addProcessor(std::shared_ptr<T> processor) {
        return static_cast<T*>(addProcessor(std::shared_ptr<Processor>(std::move(processor))));
    }

    /**
     * @brief Emplace a Processor into the ProcessorNetwork
     * @see ProcessorNetwork::addProcessor(std::shared_ptr<Processor>)
     *
     * @tparam T the processor type to add
     * @param args  Additional arguments to pass to the processor constructor.
     */
    template <typename T, class = typename std::enable_if_t<std::is_base_of_v<Processor, T>, void>,
              typename... Args>
    std::shared_ptr<T> emplaceProcessor(Args&&... args) {
        auto processor = [&]() {
            if constexpr (std::is_constructible_v<T, Args...>) {
                return std::make_shared<T>(std::forward<Args>(args)...);
            } else {
                return std::make_shared<T>(std::forward<Args>(args)..., application_);
            }
        }();

        auto name = ProcessorTraits<T>::getProcessorInfo().displayName;
        assignIdentifierAndName(*processor, name);
        if (addProcessor(processor)) {
            return processor;
        } else {
            return nullptr;
        }
    }

    ///@{
    /**
     * @brief Removes a Processor from the ProcessorNetwork.
     *
     * To ensure that the network does not end up in a corrupt state, this method first removes and
     * deletes all PortConnections and  ProcessorLinks, which are related to the Processor to be
     * removed. The processor's network and widget is also reset to nullptr.
     * @param[in] processor The Processor to be removed.
     * @returns The removed processor, or nullptr if the processor was not found
     * @see addProcessor()
     */
    std::shared_ptr<Processor> removeProcessor(Processor* processor);
    std::shared_ptr<Processor> removeProcessor(std::string_view processor);
    ///@}

    /**
     * @brief Returns the Processor from the ProcessorNetwork, which has the given identifier.
     *
     * In case no Processor with the given identifier is contained in the network, a null
     * pointer is returned.
     * @param identifier The identifier of the Processor to be accessed.
     * @returns A pointer to the processor or nullptr if there was no processor with the given @p
     *          identifier
     * @see getProcessorsByType(), Processor::setIdentifier(), Processor::getIdentifier()
     */
    Processor* getProcessorByIdentifier(std::string_view identifier) const;

    /**
     * @brief Returns a vector of Processors which are of type T.
     *
     * In case no Processors match T an empty vector is returned.
     * @note This will have to allocate a new vector, consider using forEachProcessor instead.
     * @tparam T The type of processors to look for.
     * @see getProcessorByIdentifier()
     */
    template <class T>
    std::vector<T*> getProcessorsByType() const;

    /**
     * @brief Get all processors in the network.
     * @note This will have to allocate a new vector, consider using forEachProcessor instead.
     * @returns a vector of pointers to vectors.
     */
    std::vector<Processor*> getProcessors() const;

    /**
     * @brief Apply a function to each processor in the network
     *
     * @param callback A function that matches the signature `void(Processor*)`
     */
    template <typename C>
    void forEachProcessor(C callback);

    /**
     * Returns a range of all processors (Processor&), in unspecified order.
     */
    auto processorRange() const {
        return util::transformRange(
            processors_, [](const auto& item) -> decltype(auto) { return *item.second; });
    }

    /**
     * @brief Add a PortConnection to the ProcessorNetwork.
     *
     * This involves creating the connection between the two specified ports, as well as adding this
     * connection to the ProcessorNetwork.
     * @param[in] sourcePort The outport.
     * @param[in] destPort The inport.
     * @see removeConnection()
     */
    void addConnection(Outport* sourcePort, Inport* destPort);

    /**
     * @brief Add a PortConnection to the ProcessorNetwork.
     *
     * This involves creating the connection between the two specified ports, as well as adding this
     * connection to the ProcessorNetwork.
     * @param[in] connection The connection to add
     * @see removeConnection()
     */
    void addConnection(const PortConnection& connection);

    /**
     * Check if sourcePort can be connected to destPort
     */
    bool canConnect(const Outport* sourcePort, const Inport* destPort) const;

    /**
     * @brief Removes a PortConnection from the ProcessorNetwork.
     *
     * This involves breaking the connection between the two specified Ports, as well as removing
     * this connection from the ProcessorNetwork.
     * @param[in] sourcePort The outport.
     * @param[in] destPort The inport.
     * @see addConnection()
     */
    void removeConnection(Outport* sourcePort, Inport* destPort);

    /**
     * @brief Removes a PortConnection from the ProcessorNetwork.
     *
     * This involves breaking the connection between the two specified Ports, as well as removing
     * this connection from the ProcessorNetwork.
     * @param[in] connection The connection to remove.
     * @see addConnection()
     */
    void removeConnection(const PortConnection& connection);

    /**
     * @brief Checks whether two port are connected.
     *
     * @param[in] sourcePort The outport.
     * @param[in] destPort The inport.
     * @return Weather the two port are connected
     * @see addConnection()
     */
    bool isConnected(Outport* sourcePort, Inport* destPort) const;

    /**
     * @brief Checks whether two port are connected.
     *
     * @param[in] connection The connection to look for
     * @return True if the two ports are connected
     * @see addConnection()
     */
    bool isConnected(const PortConnection& connection) const;

    /**
     * @brief Get all connections in the network
     */
    const std::vector<PortConnection>& getConnections() const;

    /**
     * @brief Apply a function to each connection in the network
     *
     * @param callback A function that matches the signature `void(PortConnection&)`
     */
    template <typename C>
    void forEachConnection(C callback);

    /**
     * Returns a range of all connections (PortConnection&), in unspecified order.
     */
    auto connectionRange() const { return util::as_range(connections_); }

    /**
     * @brief Check if @p port is owned by a processor in this ProcessorNetwork
     *
     * This will retrieve the identifier of the @p ports owner and see if this network has a
     * processor with that identifier
     */
    bool isPortInNetwork(Port* port) const;

    /**
     * @brief  Create and add Property Link to the network.
     *
     * Adds a link between two properties, that are owned by processor network.
     * @param[in] source Property at which link starts
     * @param[in] destination Property at which link ends
     */
    void addLink(Property* source, Property* destination);

    /**
     * @brief Create and add Property Link to the network.
     *
     * Adds a link between two properties, that are owned by processor network.
     * @param[in] link The property link to add.
     */
    void addLink(const PropertyLink& link);

    /**
     * @brief Check if source can be linked to destination
     */
    bool canLink(const Property* source, const Property* destination) const;

    /**
     * Remove and delete Property Link from the network
     * Removes a link between two properties, that are owned by processor network.
     * @param[in] source Property at which link starts
     * @param[in] destination Property at which link ends
     */
    void removeLink(Property* source, Property* destination);

    /**
     * Remove and delete Property Link from the network
     * Removes a link between two properties, that are owned by processor network.
     * @param[in] link The property link to remove.
     */
    void removeLink(const PropertyLink& link);

    /**
     * Check whether Property Link exists
     * Checks if there is a link between two properties, that are owned by processor network.
     * @param[in] source Property at which link starts
     * @param[in] destination Property at which link ends
     * @return True if link exists otherwise returns false
     */
    bool isLinked(Property* source, Property* destination) const;

    /**
     * Check whether Property Link exists
     * Checks if there is a link between two properties, that are owned by processor network.
     * @param[in] link The property link to look for
     * @return True if link exists otherwise returns false
     */
    bool isLinked(const PropertyLink& link) const;

    /**
     * @brief Get all connections in the network
     *
     * @note This will have to allocate a new vector, consider using forEachLink instead.
     */
    std::vector<PropertyLink> getLinks() const;

    /**
     * @brief Apply a function to each property link in the network
     *
     * @param callback A function that matched the signature `void(PropertyLink&)`
     */
    template <typename C>
    void forEachLink(C callback);

    /**
     * Returns a range of all links (PropertyLink&), in unspecified order.
     */
    auto linkRange() const { return util::as_range(links_); }

    /**
     * Is Property Link bidirectional
     * Searches for bidirectional link between start and end properties
     * In other words property that goes from end to start
     * @param[in] source Property at which link starts
     * @param[in] destination Property at which link ends
     * @return void
     */
    bool isLinkedBidirectional(Property* source, Property* destination) const;

    std::vector<Property*> getPropertiesLinkedTo(Property* property);
    std::vector<PropertyLink> getLinksBetweenProcessors(Processor* p1, Processor* p2);

    /**
     * @brief Get Property by path
     * @param path string of dot separated identifiers starting with a processor identifier followed
     * by property identifiers.
     * @return the property or nullptr if not found
     */
    Property* getProperty(std::string_view path) const;

    /**
     * @brief Get Port by path
     * @param path string of dot separated identifiers starting with a processor identifier followed
     * by a port identifier.
     * @return the port or nullptr if not found
     */
    Port* getPort(std::string_view path) const;

    /**
     * @brief Get Inport by path
     * @param path string of dot separated identifiers starting with a processor identifier followed
     * by a port identifier.
     * @return the port or nullptr if not found
     */
    Inport* getInport(std::string_view path) const;

    /**
     * @brief Get Outport by path
     * @param path string of dot separated identifiers starting with a processor identifier followed
     * by a port identifier.
     * @return the port or nullptr if not found
     */
    Outport* getOutport(std::string_view path) const;

    bool isPropertyInNetwork(Property* prop) const;

    InviwoApplication* getApplication() const;
    static int getVersion();

    void evaluateLinksFromProperty(Property*);

    bool isEmpty() const;
    bool isInvalidating() const;
    bool isLinking() const;

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

    /*
     * @return whether the network is empty, that is the network contains no processors
     */
    bool empty() const;

    /*
     * @return the number of processors in the network
     */
    size_t size() const;

    /**
     * @brief Accept a NetworkVisitor, the visitor will visit each Processor of the Network in an
     * undefined order. The Visitor will then visit each processors Properties and so on.
     */
    void accept(NetworkVisitor& visor);

    int runningBackgroundJobs() const { return backgoundJobs_; }

    static constexpr int processorNetworkVersion() { return processorNetworkVersion_; }

private:
    // Assign an identifier and display name, if none is set.
    static void assignIdentifierAndName(Processor& p, std::string_view name);
    void removeProcessorHelper(Processor* processor);

    // PropertyOwnerObserver overrides
    virtual void onWillRemoveProperty(Property* property, size_t index) override;

    // ProcessorObserver overrides.
    virtual void onAboutPropertyChange(Property*) override;
    virtual void onProcessorInvalidationBegin(Processor*) override;
    virtual void onProcessorInvalidationEnd(Processor*) override;
    virtual void onProcessorPortRemoved(Processor*, Port* port) override;

    virtual void onProcessorStartBackgroundWork(Processor*, size_t jobs) override;
    virtual void onProcessorFinishBackgroundWork(Processor*, size_t jobs) override;

    // ProcessorMeteDataObserver overrides
    virtual void onProcessorMetaDataPositionChange() override;
    virtual void onProcessorMetaDataVisibilityChange() override;
    virtual void onProcessorMetaDataSelectionChange() override;

    void addPropertyOwnerObservation(PropertyOwner*);
    void removePropertyOwnerObservation(PropertyOwner*);

    static const int processorNetworkVersion_;

    unsigned int locked_ = 0;
    bool deserializing_ = false;
    int backgoundJobs_ = 0;

    InviwoApplication* application_;

    UnorderedStringMap<std::shared_ptr<Processor>> processors_;
    std::unordered_set<PortConnection> connections_;
    // This vector is needed to keep the connections in chronological order, since some processors
    // depends on the order of connections, ideally we would get rid of this.
    std::vector<PortConnection> connectionsVec_;
    std::unordered_set<PropertyLink> links_;

    LinkEvaluator linkEvaluator_;
    std::vector<Processor*> processorsInvalidating_;

    std::unordered_map<Processor*, Processor::NameDispatcherHandle> onIdChange_;
};

template <class T>
std::vector<T*> ProcessorNetwork::getProcessorsByType() const {
    std::vector<T*> processors;
    for (const auto& [identifier, processor] : processors_) {
        if (auto p = dynamic_cast<T*>(processor.get())) processors.push_back(p);
    }
    return processors;
}

template <typename C>
void ProcessorNetwork::forEachProcessor(C callback) {
    for (auto& item : processors_) callback(item.second.get());
}

template <typename C>
void ProcessorNetwork::forEachConnection(C callback) {
    for (auto& item : connectionsVec_) callback(item);
}

template <typename C>
void ProcessorNetwork::forEachLink(C callback) {
    for (auto& item : links_) callback(item);
}

inline void ProcessorNetwork::lock() { locked_++; }
inline void ProcessorNetwork::unlock() {
    (locked_ > 0) ? locked_-- : locked_ = 0;
    if (locked_ == 0) notifyObserversProcessorNetworkUnlocked();
}
inline bool ProcessorNetwork::islocked() const { return (locked_ != 0); }

}  // namespace inviwo
