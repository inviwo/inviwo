/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_INPORT_H
#define IVW_INPORT_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/ports/port.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/util/callback.h>
#include <inviwo/core/util/statecoordinator.h>

namespace inviwo {

class Outport;
class Processor;
class ProcessorNetwork;
class ProcessorNetworkEvaluator;
class Event;

/** \class Inport
 * \brief Abstract base class for all inports
 * An Inport can be connected to an Outport.
 * The approved connection can be determined by the canConnectTo function.
 */
class IVW_CORE_API Inport : public Port {
public:
    friend class Outport;
    friend class Processor;
    friend class ProcessorNetwork;
    friend class ProcessorNetworkEvaluator;

    virtual ~Inport();

    virtual bool isConnected() const override;

    /**
     * An inport is ready when it is connected, and it's outports are ready.
     * @see Outport::isReady
     */
    virtual bool isReady() const override;

    /**
     * An inport can be set optional, in which case the processor will by default be ready even if
     * no outport is connected. This means that an optional inport may not have any connected
     * outports when evaluating the ProcessorNetwork, i.e. when Processor::process() is called.
     * Hence one needs to check manually that the port isReady before using it.
     * @see isReady
     */
    void setOptional(bool optional);
    bool isOptional() const;

    /**
     * An inport is changed when it has new data, and it's processor has not been processed.
     */
    virtual bool isChanged() const;

    // Called from the processor network to create connections.
    virtual bool canConnectTo(const Port* port) const = 0;
    virtual void connectTo(Outport* outport);
    virtual void disconnectFrom(Outport* outport);

    virtual bool isConnectedTo(const Outport* outport) const;
    virtual Outport* getConnectedOutport() const;
    virtual const std::vector<Outport*>& getConnectedOutports() const;
    virtual size_t getMaxNumberOfConnections() const = 0;
    virtual size_t getNumberOfConnections() const;
    virtual const std::vector<const Outport*>& getChangedOutports() const;

    /**
     * Propagate event upwards towards connected outports, if targets is nullptr, propagate the
     * even to all connected outport, otherwise only to target.
     */
    virtual void propagateEvent(Event* event, Outport* target = nullptr);

    // clang-format off
    /**
     * The on change call back is invoked before Processor::process after a port has been connected,
     * disconnected, or has changed its validation level. Note it is only called if process is also
     * going to be called.
     */
    template <typename T>
    [[deprecated("was declared deprecated. Use `onChange(std::function<void()>)` instead")]]
    const BaseCallBack* onChange(T* o, void (T::*m)());
    const BaseCallBack* onChange(std::function<void()> lambda);

    /**
     *    the onInvalid callback is called directly after the port has been invalidated. It's only
     *    called once for each transition from valid to invalid.
     */
    template <typename T>
    [[deprecated("was declared deprecated. Use `onInvalid(std::function<void()>)` instead")]]
    const BaseCallBack* onInvalid(T* o, void (T::*m)());
    const BaseCallBack* onInvalid(std::function<void()> lambda);

    const BaseCallBack* onConnect(std::function<void()> lambda);
    const BaseCallBack* onDisconnect(std::function<void()> lambda);

    void removeOnChange(const BaseCallBack* callback);
    template <typename T>
    [[deprecated("was declared deprecated. Use `removeOnChange(const BaseCallBack*)` instead")]]
    void removeOnChange(T* o);

    void removeOnInvalid(const BaseCallBack* callback);
    template <typename T>
    [[deprecated("was declared deprecated. Use `removeOnInvalid(const BaseCallBack*)` instead")]]
    void removeOnInvalid(T* o);

    // clang-format on

    void removeOnConnect(const BaseCallBack* callback);
    void removeOnDisconnect(const BaseCallBack* callback);

    /**
     * Called by the connected outports to let the inport know that their ready status has
     * changed.
     */
    void readyUpdate();
    void setIsReadyUpdater(std::function<bool()> updater);

protected:
    Inport(std::string identifier = "");

    /**
     * Called by Outport::invalidate on its connected inports, which is call by
     * Processor::invalidate. Will by default invalidate its processor. From above in the
     * network.
     */
    virtual void invalidate(InvalidationLevel invalidationLevel);
    /**
     * Called by Outport::setValid, which is call by Processor::setValid, which is called after
     * Processor:process. From above in the network.
     */
    virtual void setValid(const Outport* source);

    // Usually called with false (reset) by Processor::setValid after the Processor::process
    virtual void setChanged(bool changed = true, const Outport* source = nullptr);

    // Called by the processor network.
    void callOnChangeIfChanged() const;

    StateCoordinator<bool> isReady_;
    StateCoordinator<bool> isOptional_;
    std::vector<Outport*> connectedOutports_;

private:
    bool changed_;

    CallBackList onChangeCallback_;
    std::vector<const Outport*> changedSources_;

    CallBackList onInvalidCallback_;
    InvalidationLevel lastInvalidationLevel_;  // Used for the onInvalid callback.

    CallBackList onConnectCallback_;
    CallBackList onDisconnectCallback_;
};

// clang-format off

template <typename T>
[[deprecated("was declared deprecated. Use `onChange(std::function<void()>)` instead")]]
const BaseCallBack* Inport::onChange(T* o, void (T::*m)()) {
    return onChangeCallback_.addMemberFunction(o, m);
}

template <typename T>
[[deprecated("was declared deprecated. Use `onInvalid(std::function<void()>)` instead")]]
const BaseCallBack* Inport::onInvalid(T* o, void (T::*m)()) {
    return onInvalidCallback_.addMemberFunction(o, m);
}

template <typename T>
[[deprecated("was declared deprecated. Use `removeOnChange(const BaseCallBack*)` instead")]]
void Inport::removeOnChange(T* o) {
    onChangeCallback_.removeMemberFunction(o);
}

template <typename T>
[[deprecated("was declared deprecated. Use `removeOnInvalid(const BaseCallBack*)` instead")]]
void Inport::removeOnInvalid(T* o) {
    onInvalidCallback_.removeMemberFunction(o);
}

// clang-format on

}  // namespace inviwo

#endif  // IVW_INPORT_H