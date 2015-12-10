/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_OUTPORT_H
#define IVW_OUTPORT_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/ports/port.h>

namespace inviwo {

class Inport;
class Processor;

/**
 * \class Outport 
 * \brief Abstract base class for all outports
 * The Outport can be connected to an arbitrary number of Inports.
 */
class IVW_CORE_API Outport : public Port {
    friend class Inport;
    friend class Processor;

public:
    virtual ~Outport();
    virtual bool isConnected() const override;

    /**
     * An outport is ready if it has data and is valid.
     */
    virtual bool isReady() const override = 0;

    /**
     *    Called by Processor::invalidate, will invalidate its connected inports.
     */
    virtual void invalidate(InvalidationLevel invalidationLevel);
    virtual InvalidationLevel getInvalidationLevel() const;
    
    /**
     *  Propagate events upwards, i.e. to the owning processor.
     */
    virtual void propagateEvent(Event* event);

    bool isConnectedTo(const Inport* port) const;
    const std::vector<Inport*>& getConnectedInports() const;

    /**
     * Called each time connected to an inport.
     */
    const BaseCallBack* onConnect(std::function<void()> lambda);
    /** 
     * Called each time disconnected from an inport.
     */
    const BaseCallBack* onDisconnect(std::function<void()> lambda);
    void removeOnConnect(const BaseCallBack* callback);
    void removeOnDisconnect(const BaseCallBack* callback);

protected:
    Outport(std::string identifier = "");
    /**
    *    Called by Processor::setValid, will call setValid its connected inports.
    */
    virtual void setValid();

    // These function are only called by the corresponding inport.
    virtual void connectTo(Inport* port);
    virtual void disconnectFrom(Inport* port);

    InvalidationLevel invalidationLevel_;
    std::vector<Inport*> connectedInports_;

    CallBackList onConnectCallback_;
    CallBackList onDisconnectCallback_;
};

}  // namespace

#endif  // IVW_OUTPORT_H