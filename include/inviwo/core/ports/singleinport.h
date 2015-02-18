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

#ifndef IVW_SINGLE_INPORT_H
#define IVW_SINGLE_INPORT_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/ports/inport.h>

namespace inviwo {

class Outport;

/**
 * \class SingleInport
 *
 * \brief The SingleInport can be connected to only one outport at a time.
 */
class IVW_CORE_API SingleInport : public Inport {

public:
    /**
     * @param invalidationLevel Defines the level of invalidation used upon connection/deconnection.
     * @see Processor::addPort(), InvalidationLevel
     */
    SingleInport(std::string identifier,
                 InvalidationLevel invalidationLevel=INVALID_OUTPUT);
    virtual ~SingleInport();

    virtual InvalidationLevel getInvalidationLevel() const;
    virtual void setInvalidationLevel(InvalidationLevel invalidationLevel);

    virtual void connectTo(Outport* outport);
    virtual void disconnectFrom(Outport* outport);

    virtual bool isConnected() const;
    bool isConnectedTo(Outport* outport) const;

    Outport* getConnectedOutport() const { return connectedOutport_; }
    std::vector<Outport*> getConnectedOutports() const { return std::vector<Outport*>(1, connectedOutport_); }

    virtual void invalidate(InvalidationLevel invalidationLevel);

    template <typename T>
    void onInvalid(T* o, void (T::*m)(), bool add = true) {
        if(add)
            onInvalidCallback_.addMemberFunction(o,m);
        else
            onInvalidCallback_.removeMemberFunction(o);
    }

protected:
    Outport* connectedOutport_;
    InvalidationLevel invalidationLevel_;

    CallBackList onInvalidCallback_;
};



} // namespace

#endif // IVW_SINGLE_INPORT_H