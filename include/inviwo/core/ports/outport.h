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

class SingleInport;
class MultiInport;
class Inport;

/**
 * \class Outport
 *
 * \brief The Outport can be connected to an arbitary number of Inports.
 */
class IVW_CORE_API Outport : public Port {

    friend class SingleInport;
    friend class MultiInport;
    friend class ImageInport;
public:
    Outport(std::string identifier = "",
            InvalidationLevel invalidationLevel=INVALID_OUTPUT);
    virtual ~Outport();

    //TODO: Temporary fix to enable Deserialization. Remove this to make Outport abstract class
    virtual void initialize() {}
    virtual void deinitialize() {}

    virtual bool isConnected() const;
    bool isConnectedTo(Inport* port) const;

    virtual bool isReady() const { return isConnected(); }

    bool isValid() { return (getInvalidationLevel() == VALID); }

    std::vector<Inport*> getConnectedInports() const { return connectedInports_; }

    virtual void invalidate(InvalidationLevel invalidationLevel);
    void invalidateConnectedInports(InvalidationLevel invalidationLevel);

    virtual InvalidationLevel getInvalidationLevel() const { return invalidationLevel_; }
    virtual void setInvalidationLevel(InvalidationLevel invalidationLevel);

    std::vector<Processor*> getDirectSuccessors();

    virtual std::string getClassIdentifier() const {return "org.inviwo.Outport";}
    virtual std::string getContentInfo() const {return "";}

protected:
    void connectTo(Inport* port);
    void disconnectFrom(Inport* port);

    template <typename T>
    void getSuccessorsUsingPortType(std::vector<Processor*>&);

    InvalidationLevel invalidationLevel_;
private:
    std::vector<Inport*> connectedInports_;
};

} // namespace

#endif // IVW_OUTPORT_H