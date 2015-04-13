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

#ifndef IVW_MULTI_DATA_INPORT_H
#define IVW_MULTI_DATA_INPORT_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/multiinport.h>
#include <inviwo/core/ports/vectordataport.h>
#include <set>

namespace inviwo {

template<typename T>
class DataOutport;

/** 
 * \class MultiDataInport
 *
 * \brief Port for handling multiple inports.
 *
 * First template argument is the data contained within
 * the ports and the second one is the port class.
 * Usage example:
 * MultiDataInport<Image, ImagePort> multiImagePort;
 * MultiDataInport<float> multiFloatPort;
 * @see MultiInport
 */
template < typename T, typename U = DataInport<T> >
class MultiDataInport : public MultiInport {

public:
    typedef std::vector< U* > DataInportVec;

    MultiDataInport(std::string identifier);
    virtual ~MultiDataInport();

    virtual bool canConnectTo(Port* port) const {
        if(!port || port->getProcessor() == getProcessor())
            return false;
        if (dynamic_cast<DataOutport<T>*>(port))
            return true;
        else if (dynamic_cast<VectorDataOutport<T*>*>(port))
            return true;
        else
            return false;
    };
    virtual void connectTo(Outport* outport);

    uvec3 getColorCode() const { return U::colorCode; }

    std::vector<const T*> getData() const;

    std::vector<const T*> getDataFromPort(Inport*) const;

    bool hasData() const;

};


template<typename T, typename U>
MultiDataInport<T, U>::MultiDataInport(std::string identifier)
    : MultiInport(identifier) {
}

template <typename T, typename U>
MultiDataInport<T, U>::~MultiDataInport() {
}

template <typename T, typename U>
void MultiDataInport<T, U>::connectTo(Outport* outport) {
    ivwAssert(dynamic_cast<DataOutport<T>*>(outport)!=nullptr
              || dynamic_cast<VectorDataOutport<T*>*>(outport) != nullptr
              , "Trying to connect incompatible ports.")
    // U is a Port class
    Inport* inport = nullptr;
    if (dynamic_cast<DataOutport<T>*>(outport)) {
        inport = new U(getIdentifier());
        inports_->push_back(inport);
    } else if (dynamic_cast<VectorDataOutport<T*>*>(outport)) {
        inport = new VectorDataInport<T*>(getIdentifier());
        vectorInports_->push_back(inport);
    }
    setProcessorHelper(inport, getProcessor());
    inport->connectTo(outport);
    inport->setChanged(true);
    invalidate(INVALID_OUTPUT);
}

template < typename T, typename U /*= DataInport<T> */>
std::vector<const T*> inviwo::MultiDataInport<T, U>::getData() const {
    std::vector<const T*> data;
    InportVec::const_iterator it = inports_->begin();
    InportVec::const_iterator endIt = inports_->end();

    for (; it != endIt; ++it) {
        data.push_back(static_cast<U*>(*it)->getData());
    }
    
    it = vectorInports_->begin();
    endIt = vectorInports_->end();
    for (; it != endIt; ++it) {
        const VectorData<T*>* vecdata = static_cast<VectorDataInport<T*>*>(*it)->getData();
        data.insert(data.end(), vecdata->vector.begin(), vecdata->vector.end());
    }

    return data;
}

template < typename T, typename U /*= DataInport<T> */>
std::vector<const T*> inviwo::MultiDataInport<T, U>::getDataFromPort(Inport* port) const {
    std::vector<const T*> data;
    InportVec::const_iterator it = inports_->begin();
    InportVec::const_iterator endIt = inports_->end();

    for (; it != endIt; ++it) {
        if ((*it) == port){
            data.push_back(static_cast<U*>(*it)->getData());
            return data;
        }
    }

    it = vectorInports_->begin();
    endIt = vectorInports_->end();
    for (; it != endIt; ++it) {
        if ((*it) == port){
            const VectorData<T*>* vecdata = static_cast<VectorDataInport<T*>*>(*it)->getData();
            data.insert(data.end(), vecdata->vector.begin(), vecdata->vector.end());
            return data;
        }
    }

    return data;
}

template < typename T, typename U /*= DataInport<T> */>
bool inviwo::MultiDataInport<T, U>::hasData() const {
    if (isConnected()) {
        InportVec::const_iterator it = inports_->begin();
        InportVec::const_iterator endIt = inports_->end();

        for (; it != endIt; ++it) {
            if (!static_cast<U*>(*it)->hasData())
                return false;
        }

        it = vectorInports_->begin();
        endIt = vectorInports_->end();
        for (; it != endIt; ++it) {
            if (!static_cast<VectorDataInport<T*>*>(*it)->hasData())
                return false;
        }

        return true;
    } else
        return false;
}






} // namespace

#endif // IVW_MULTI_DATA_INPORT_H