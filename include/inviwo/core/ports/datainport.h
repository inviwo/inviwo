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

#ifndef IVW_DATAINPORT_H
#define IVW_DATAINPORT_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/datastructures/data.h>
#include <inviwo/core/util/stdextensions.h>
#include <limits>

namespace inviwo {

template<typename T>
class DataOutport;

template <typename T, size_t N = 1>
class DataInport : public Inport {
public:
    DataInport(std::string identifier);
    virtual ~DataInport();

    virtual uvec3 getColorCode() const override;
    virtual std::string getClassIdentifier() const override;
    virtual std::string getContentInfo() const override;
    virtual size_t getMaxNumberOfConnections() const override;

    virtual bool canConnectTo(const Port* port) const override;
    virtual void connectTo(Outport* port) override;
    virtual bool isConnected() const override;
    virtual bool isReady() const override;

    virtual const T* getData() const;
    virtual std::vector<const T*> getVectorData() const;
    virtual std::vector<std::pair<const Outport*, const T*>> getSourceVectorData() const;

    bool hasData() const;
};


template <typename T, size_t N>
DataInport<T, N>::DataInport(std::string identifier)
    : Inport(identifier) {
}

template <typename T, size_t N>
DataInport<T, N>::~DataInport() {}

template <typename T, size_t N>
std::string inviwo::DataInport<T, N>::getClassIdentifier() const {
    return port_traits<T>::class_identifier() + (N == 0 ? "Multi" : "") + "Inport";
}

template <typename T, size_t N>
uvec3 DataInport<T, N>::getColorCode() const { return port_traits<T>::color_code(); }

template <typename T, size_t N>
size_t inviwo::DataInport<T, N>::getMaxNumberOfConnections() const  {
    if (N==0) return std::numeric_limits<size_t>::max();
    else return N;
}

template <typename T, size_t N>
bool DataInport<T, N>::canConnectTo(const Port* port) const {
    if (dynamic_cast<const DataOutport<T>*>(port) && port->getProcessor() != getProcessor())
        return true;
    else
        return false;
}

template <typename T, size_t N>
void DataInport<T, N>::connectTo(Outport* port) {
    if (!port) return;

    DataOutport<T>* dataPort = dynamic_cast<DataOutport<T>*>(port);
    if (!dataPort) throw Exception("Trying to connect incompatible ports.", IvwContext);

    if (getNumberOfConnections() + 1 > getMaxNumberOfConnections()) 
        throw Exception("Trying to connect to a full port.", IvwContext);

    Inport::connectTo(port);
}

template <typename T, size_t N>
bool inviwo::DataInport<T, N>::isConnected() const  {
    if(N==0) return !connectedOutports_.empty();
    else return connectedOutports_.size() == N;
}

template <typename T, size_t N>
bool DataInport<T, N>::isReady() const {
    return isConnected() && hasData() && util::all_of(connectedOutports_, [](Outport* p) {
                                             return p->getInvalidationLevel() == VALID;
                                         });
}

template <typename T, size_t N>
const T* DataInport<T, N>::getData() const {
    if (isConnected()) {
        // Safe to static cast since we are unable to connect other outport types.
        return static_cast<const DataOutport<T>*>(getConnectedOutport())->getConstData();
    } else {
        return nullptr;
    }
}

template <typename T, size_t N>
std::vector<const T*> DataInport<T, N>::getVectorData() const {
    std::vector<const T*> res(N);

    for (auto outport : connectedOutports_) {
        // Safe to static cast since we are unable to connect other outport types.
        auto dataport = static_cast<const DataOutport<T>*>(outport);
        if (dataport->hasData()) res.push_back(dataport->getConstData());
    }

    return res;
}


template <typename T, size_t N>
std::vector<std::pair<const Outport*, const T*>> inviwo::DataInport<T, N>::getSourceVectorData() const {
    std::vector<std::pair<const Outport*, const T*>> res(N);

    for (auto outport : connectedOutports_) {
        // Safe to static cast since we are unable to connect other outport types.
        auto dataport = static_cast<const DataOutport<T>*>(outport);
        if (dataport->hasData()) res.emplace_back(dataport, dataport->getConstData());
    }

    return res;
}

template <typename T, size_t N>
bool DataInport<T, N>::hasData() const {
    return isConnected() && util::all_of(connectedOutports_, [](Outport* p) {
                                return static_cast<DataOutport<T>*>(p)->hasData();
                            });
}

template <typename T, size_t N>
std::string DataInport<T, N>::getContentInfo() const {
    if (hasData()) {
        std::string info = port_traits<T>::data_info(getData());
        if (!info.empty()) {
            return info;
        } else {
            return "No information available for: " + util::class_identifier<T>();
        }
    } else {
        return "Port has no data";
    }
}

} // namespace

#endif // IVW_DATAINPORT_H
