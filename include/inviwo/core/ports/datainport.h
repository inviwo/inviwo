/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/ports/porttraits.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/ports/outportiterable.h>
#include <inviwo/core/ports/inportiterable.h>
#include <inviwo/core/datastructures/datatraits.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/document.h>

#include <memory>
#include <vector>

namespace inviwo {

/**
 * \ingroup ports
 * DataInport represents a general inport providing data as a std:shared_ptr<const T>
 * If N is set to 0 the port will accept multiple connections, and will provide a
 * std::vector<std::shared_ptr<const T>> of data. If N is larger then 1 exactly that many
 * connections are accepted. If Flat is set to true, the inport will also accept connections from
 * outport with vector data of type T and merge them into the data return data vector.
 */
template <typename T, size_t N = 1, bool Flat = false>
class DataInport : public Inport, public InportIterable<DataInport<T, N, Flat>, T, Flat> {
public:
    using type = T;
    using value_type = std::shared_ptr<const T>;
    static constexpr bool flattenData = Flat;
    static constexpr size_t maxConnections = N;

    DataInport(std::string_view identifier, Document help = {});
    virtual ~DataInport() = default;

    virtual std::string getClassIdentifier() const override;
    virtual uvec3 getColorCode() const override;
    virtual Document getInfo() const override;

    virtual size_t getMaxNumberOfConnections() const override;

    virtual bool canConnectTo(const Port* port) const override;
    virtual void connectTo(Outport* port) override;
    virtual bool isConnected() const override;

    virtual std::shared_ptr<const T> getData() const;
    virtual std::vector<std::shared_ptr<const T>> getVectorData() const;
    virtual std::vector<std::pair<Outport*, std::shared_ptr<const T>>> getSourceVectorData() const;

    virtual bool hasData() const;
};

template <typename T>
using MultiDataInport = DataInport<T, 0, false>;

template <typename T>
using FlatMultiDataInport = DataInport<T, 0, true>;

template <typename T, size_t N, bool Flat>
struct PortTraits<DataInport<T, N, Flat>> {
    static std::string classIdentifier() {
        auto&& classId = DataTraits<T>::classIdentifier();
        if (classId.empty()) return {};

        StrBuffer name;
        name.append("{}", classId);
        if constexpr (Flat) {
            name.append(".flat");
        }
        if constexpr (N == 0) {
            name.append(".multi");
        } else if constexpr (N != 1) {
            name.append(".{}", N);
        }
        name.append(".inport");
        return std::string{name.view()};
    }
};

template <typename T, size_t N, bool Flat>
DataInport<T, N, Flat>::DataInport(std::string_view identifier, Document help)
    : Inport(identifier, std::move(help)), InportIterable<DataInport<T, N, Flat>, T, Flat>{} {}

template <typename T, size_t N, bool Flat>
std::string DataInport<T, N, Flat>::getClassIdentifier() const {
    return PortTraits<DataInport<T, N, Flat>>::classIdentifier();
}

template <typename T, size_t N, bool Flat>
uvec3 DataInport<T, N, Flat>::getColorCode() const {
    return DataTraits<T>::colorCode();
}

template <typename T, size_t N, bool Flat>
size_t DataInport<T, N, Flat>::getMaxNumberOfConnections() const {
    if constexpr (N == 0) {
        return std::numeric_limits<size_t>::max();
    } else {
        return N;
    }
}

template <typename T, size_t N, bool Flat>
bool DataInport<T, N, Flat>::canConnectTo(const Port* port) const {
    if (!port || port->getProcessor() == getProcessor() || circularConnection(port)) return false;

    if constexpr (Flat) {
        if (dynamic_cast<const OutportIterable<T>*>(port)) {
            return true;
        }
    }

    if (dynamic_cast<const DataOutport<T>*>(port)) {
        return true;
    }

    return false;
}

template <typename T, size_t N, bool Flat>
void DataInport<T, N, Flat>::connectTo(Outport* port) {
    if (!port) return;

    const DataOutport<T>* dataPort = dynamic_cast<const DataOutport<T>*>(port);
    const OutportIterable<T>* flatPort =
        Flat ? dynamic_cast<const OutportIterable<T>*>(port) : nullptr;

    if (dataPort == nullptr && flatPort == nullptr)
        throw Exception("Trying to connect incompatible ports.", IVW_CONTEXT);

    if (getNumberOfConnections() + 1 > getMaxNumberOfConnections())
        throw Exception("Trying to connect to a full port.", IVW_CONTEXT);

    Inport::connectTo(port);
}

template <typename T, size_t N, bool Flat>
bool DataInport<T, N, Flat>::isConnected() const {
    if constexpr (N == 0) {
        return !connectedOutports_.empty();
    } else {
        return connectedOutports_.size() >= 1 && connectedOutports_.size() <= N;
    }
}

template <typename T, size_t N, bool Flat>
bool DataInport<T, N, Flat>::hasData() const {
    if constexpr (N == 0) {
        return isConnected() && this->begin() != this->end();
    } else {
        return isConnected() && util::all_of(connectedOutports_, [](Outport* p) {
                   return static_cast<DataOutport<T>*>(p)->hasData();
               });
    }
}

template <typename T, size_t N, bool Flat>
std::shared_ptr<const T> DataInport<T, N, Flat>::getData() const {
    if (isConnected()) {
        auto it = this->begin();
        if (it != this->end()) return *it;
    }
    return nullptr;
}

template <typename T, size_t N, bool Flat>
std::vector<std::shared_ptr<const T>> DataInport<T, N, Flat>::getVectorData() const {
    std::vector<std::shared_ptr<const T>> res;
    if constexpr (N > 0) {
        res.reserve(N);
    }

    for (auto it = this->begin(); it != this->end(); ++it) {
        res.push_back(it.operator->());
    }

    return res;
}

template <typename T, size_t N, bool Flat>
std::vector<std::pair<Outport*, std::shared_ptr<const T>>>
DataInport<T, N, Flat>::getSourceVectorData() const {
    std::vector<std::pair<Outport*, std::shared_ptr<const T>>> res;
    if constexpr (N > 0) {
        res.reserve(N);
    }

    for (auto outport : connectedOutports_) {
        // Safe to static cast since we are unable to connect other outport types.
        if (Flat) {
            if (auto iterable = dynamic_cast<OutportIterable<T>*>(outport)) {
                for (auto elem : *iterable) res.emplace_back(outport, elem);
            }
        } else {
            auto dataport = static_cast<DataOutport<T>*>(outport);
            if (dataport->hasData()) res.emplace_back(dataport, dataport->getData());
        }
    }

    return res;
}

template <typename T, size_t N, bool Flat>
Document DataInport<T, N, Flat>::getInfo() const {
    StrBuffer name;
    name.append("{}", util::htmlEncode(DataTraits<T>::dataName()));
    if constexpr (Flat) {
        name.append(" Flat");
    }
    if constexpr (N == 0) {
        name.append(" Multi");
    } else if constexpr (N != 1) {
        name.append(" {}", N);
    }
    name.append(" Inport");

    Document doc;
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    doc.append("b", name.view(), {{"class", "name"}});

    if (!help_.empty()) {
        doc.append("div", "", {{"class", "help"}}).append(help_);
    }

    utildoc::TableBuilder tb(doc.handle(), P::end());
    tb(H("Identifier"), getIdentifier());
    tb(H("Class"), getClassIdentifier());
    tb(H("Ready"), isReady());
    tb(H("Connected"), isConnected());

    tb(H("Connections"),
       fmt::format("{} ({})", getNumberOfConnections(), getMaxNumberOfConnections()));
    tb(H("Optional"), isOptional());

    if (hasData()) {
        doc.append("p").append(DataTraits<T>::info(*getData()));
    } else {
        doc.append("p", "Port has no data");
    }
    return doc;
}

}  // namespace inviwo
