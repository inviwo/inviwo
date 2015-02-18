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

#ifndef IVW_INPORT_H
#define IVW_INPORT_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/ports/port.h>
#include <inviwo/core/util/callback.h>

namespace inviwo {

class Outport;

/**
 * \class Inport
 * \brief An Inport can be connected to an Outport.
 * The approved connection can be determined by the canConnectTo function.
 */
class IVW_CORE_API Inport : public Port {

public:
    Inport(std::string identifier = "");
    virtual ~Inport();

    //FIXME: Temporary fix. Remove this to make Inport abstract class
    virtual void initialize() {}
    virtual void deinitialize() {}

    virtual bool isConnected() const;
    virtual bool isReady() const;

    virtual void invalidate(InvalidationLevel invalidationLevel);
    virtual void setInvalidationLevel(InvalidationLevel invalidationLevel) {};

    virtual bool canConnectTo(Port* port) const { return false; }
    virtual void connectTo(Outport* outport) {};
    virtual void disconnectFrom(Outport* outport) {};

    virtual bool isConnectedTo(Outport* outport) const { return false; }

    virtual Outport* getConnectedOutport() const { return NULL; }
    virtual std::vector<Outport*> getConnectedOutports() const { return std::vector<Outport*>(); }

    std::vector<Processor*> getPredecessors();

    virtual std::string getClassIdentifier() const {return "org.inviwo.Inport";}
    virtual std::string getContentInfo() const {return "";}

    template <typename T>
    void onChange(T* o, void (T::*m)()) const {
        onChangeCallback_.addMemberFunction(o,m);
    }

    template <typename T>
    void removeOnChange(T* o) const {
        onChangeCallback_.removeMemberFunction(o);
    }

    void callOnChangeIfChanged();
    virtual bool isChanged();
    virtual void setChanged(bool changed = true);

protected:
    template <typename T>
    void getPredecessorsUsingPortType(std::vector<Processor*>&);

    mutable CallBackList onChangeCallback_;
    bool changed_;

};

} // namespace

#endif // IVW_INPORT_H