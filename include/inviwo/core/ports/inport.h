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

    virtual bool isConnected() const override = 0;
    virtual bool isReady() const override;

    virtual void invalidate(InvalidationLevel invalidationLevel) override;
    virtual void setInvalidationLevel(InvalidationLevel invalidationLevel) override = 0;
    virtual InvalidationLevel getInvalidationLevel() const override = 0;

    virtual bool canConnectTo(Port* port) const = 0;
    virtual void connectTo(Outport* outport) = 0;
    virtual void disconnectFrom(Outport* outport) = 0;
    virtual bool isConnectedTo(Outport* outport) const = 0;
    virtual Outport* getConnectedOutport() const = 0;
    virtual std::vector<Outport*> getConnectedOutports() const = 0;
    std::vector<Processor*> getPredecessors() const;

    template <typename T>
    void onChange(T* o, void (T::*m)()) const;
    template <typename T>
    void removeOnChange(T* o) const;

    void callOnChangeIfChanged() const;

    virtual bool isChanged() const;
    virtual void setChanged(bool changed = true);

protected:
    template <typename T>
    void getPredecessorsUsingPortType(std::vector<Processor*>&) const;

    mutable CallBackList onChangeCallback_;
    bool changed_;
};

template <typename T>
void Inport::onChange(T* o, void (T::*m)()) const {
    onChangeCallback_.addMemberFunction(o, m);
}

template <typename T>
void Inport::removeOnChange(T* o) const {
    onChangeCallback_.removeMemberFunction(o);
}

template <typename T>
void Inport::getPredecessorsUsingPortType(std::vector<Processor*>& predecessorsProcessors) const {
    if (isConnected()) {
        std::vector<Outport*> connectedOutports = getConnectedOutports();
        std::vector<Outport*>::const_iterator it = connectedOutports.begin();
        std::vector<Outport*>::const_iterator endIt = connectedOutports.end();

        for (; it != endIt; ++it) {
            Processor* predecessorsProcessor = (*it)->getProcessor();

            if (std::find(predecessorsProcessors.begin(), predecessorsProcessors.end(),
                          predecessorsProcessor) == predecessorsProcessors.end())
                predecessorsProcessors.push_back(predecessorsProcessor);

            std::vector<Inport*> inports = predecessorsProcessor->getInports();

            for (auto& inport : inports) {
                T* inPort = dynamic_cast<T*>(inport);

                if (inPort)
                    inPort->template getPredecessorsUsingPortType<T>(predecessorsProcessors);
            }
        }
    }
}

} // namespace

#endif // IVW_INPORT_H