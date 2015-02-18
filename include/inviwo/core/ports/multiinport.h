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

#ifndef IVW_MULTI_INPORT_H
#define IVW_MULTI_INPORT_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/ports/inport.h>
#include <set>

namespace inviwo {

/** \class MultiInport
 *
 * \brief Support class for inports that can handle multiple inputs.
 *
 * This class allows us to check if an Inport
 * can handle multiple connections through dynamic_cast.
 * @see MultiDataInport
 */
class IVW_CORE_API MultiInport : public Inport {
public:
    typedef std::vector< Inport* > InportVec;
    MultiInport(std::string identifier);
    virtual ~MultiInport();

    virtual void invalidate(InvalidationLevel invalidationLevel);

    bool isConnected() const { return !inports_->empty() || !vectorInports_->empty(); }

    bool isConnectedTo(Outport* outport) const;

    virtual InvalidationLevel getInvalidationLevel() const;
    virtual void setInvalidationLevel(InvalidationLevel invalidationLevel);
    virtual void setChanged(bool changed = true);
    virtual bool isChanged();

    std::vector<Inport*> getInports() const;

    virtual Outport* getConnectedOutport() const;
    std::vector<Outport*> getConnectedOutports() const;
    size_t getNumConnectedOutports() const;

    void disconnectFrom(Outport* outport);

protected:

    /**
     * Derived classes of Port will not have access to Port::setProcessor.
     * To make it simeple and avoid template nightmares
     * this class is instead friend of Port. This method
     * allows MultiDataInport<T,U> classes
     * to call setProcessor on a port through this helper function.
     * This method simply calls port->setProcessor
     * @param port
     * @param processor
     */
    void setProcessorHelper(Port* port, Processor* processor);

    InportVec* inports_;
    InportVec* vectorInports_;

	size_t numConnections_;
};

} // namespace

#endif // IVW_MULTI_INPORT_H