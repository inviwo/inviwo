/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2026 Inviwo Foundation
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
#include <inviwo/core/ports/inportiterable.h>
#include <inviwo/core/datastructures/datatraits.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/document.h>

#include <memory>
#include <vector>
#include <fmt/compile.h>

namespace inviwo {

/**
 * @ingroup ports
 * DataInport represents a general inport providing data as a std:shared_ptr<const T>
 * If @tparam N is set to 0 the port will accept multiple connections, and will provide a
 * std::vector<std::shared_ptr<const T>> of data. If @tparam N is greater or equal to 0 exactly
 * @tparam N connections are accepted.
 * If @tparam Flat is set to true, the inport will also accept
 * connections from outports with vector data of type @tparam T and merge them into the vector
 * returned by @c getData().
 */
template <typename T, size_t N = 1, bool Flat = false>
class DataInport : public Inport {
public:
    using type = T;
    using value_type = std::shared_ptr<const T>;
    static constexpr bool flattenData = Flat;
    static constexpr size_t maxConnections = N;

    DataInport(std::string_view identifier, Document help = {});
    DataInport(const DataInport&) = delete;
    DataInport(DataInport&&) = delete;
    DataInport& operator=(const DataInport&) = delete;
    DataInport& operator=(DataInport&&) = delete;
    virtual ~DataInport() = default;

    virtual std::string_view getClassIdentifier() const override;
    virtual uvec3 getColorCode() const override;
    virtual Document getInfo() const override;

    virtual Outport* getConnectedOutport(size_t i) const override;
    virtual size_t getMaxNumberOfConnections() const override;
    virtual size_t getNumberOfConnections() const override;
    using Inport::getConnectedOutport;

    virtual bool canConnectTo(const Port* port) const override;
    virtual void connectTo(Outport* port) override;
    virtual void disconnectFrom(Outport* outport) override;
    virtual bool isConnected() const override;

    virtual std::shared_ptr<const T> getData() const;
    virtual std::vector<std::shared_ptr<const T>> getVectorData() const;
    virtual std::vector<std::pair<Outport*, std::shared_ptr<const T>>> getSourceVectorData() const;

    virtual bool hasData() const;

    virtual DataInfo getDataInfo() const override;

    using iterator = std::conditional_t<Flat, FlatInportIterator<T>, RegularInportIterator<T>>;
    iterator begin() const noexcept { return iterator{outports_.begin(), outports_.end()}; }
    iterator end() const noexcept { return iterator{outports_.end(), outports_.end()}; }

protected:
    std::vector<DataOutportInterface<T>*> outports_;
};

template <typename T>
using MultiDataInport = DataInport<T, 0, false>;

template <typename T>
using FlatMultiDataInport = DataInport<T, 0, true>;

template <typename T, size_t N, bool Flat>
struct PortTraits<DataInport<T, N, Flat>> {
    static constexpr auto cld = []() {
        constexpr auto tCid = DataTraits<T>::classIdentifier();
        if constexpr (tCid.empty()) {
            return StaticString{};
        }

        constexpr auto flat = []() {
            if constexpr (Flat) {
                return StaticString{".flat"};
            } else {
                return StaticString{};
            }
        }();
        constexpr auto multi = []() {
            if constexpr (N == 0) {
                return StaticString{".multi"};
            } else if constexpr (N != 1) {
                return util::toStaticString<N>(IVW_COMPILE_STRING("{.}"));
            } else {
                return StaticString{};
            }
        }();

        return StaticString<tCid.size()>(tCid) + flat + multi + ".inport";
    }();
    static constexpr std::string_view classIdentifier() { return cld; }
};

template <typename T, size_t N, bool Flat>
DataInport<T, N, Flat>::DataInport(std::string_view identifier, Document help)
    : Inport(identifier, std::move(help)) {}

template <typename T, size_t N, bool Flat>
std::string_view DataInport<T, N, Flat>::getClassIdentifier() const {
    return PortTraits<DataInport<T, N, Flat>>::classIdentifier();
}

template <typename T, size_t N, bool Flat>
uvec3 DataInport<T, N, Flat>::getColorCode() const {
    return DataTraits<T>::colorCode();
}

template <typename T, size_t N, bool Flat>
Outport* DataInport<T, N, Flat>::getConnectedOutport(size_t i) const {
    if (i < outports_.size()) {
        return outports_[i]->port();
    } else {
        return nullptr;
    }
}

template <typename T, size_t N, bool Flat>
size_t DataInport<T, N, Flat>::getNumberOfConnections() const {
    return outports_.size();
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

    if (auto* outportData = dynamic_cast<const DataOutportInterface<T>*>(port)) {
        if constexpr (!Flat) {
            if (outportData->flat()) return false;
        }
        return true;
    }

    return false;
}

template <typename T, size_t N, bool Flat>
void DataInport<T, N, Flat>::connectTo(Outport* outport) {
    if (!outport) return;
    if (isConnectedTo(outport)) return;

    if (auto* outportData = dynamic_cast<DataOutportInterface<T>*>(outport)) {
        if constexpr (!Flat) {
            if (outportData->flat()) {
                throw Exception("Trying to connect incompatible ports.");
            }
        }

        if (getNumberOfConnections() + 1 > getMaxNumberOfConnections()) {
            throw Exception("Trying to connect to a full port.");
        }
        outports_.push_back(outportData);

        doConnectTo(outport);
    } else {
        throw Exception("Trying to connect incompatible ports.");
    }
}

template <typename T, size_t N, bool Flat>
void DataInport<T, N, Flat>::disconnectFrom(Outport* outport) {
    if (auto it = std::ranges::find_if(outports_, [&](auto* dp) { return dp->port() == outport; });
        it != outports_.end()) {

        outports_.erase(it);
        doDisconnectFrom(outport);
    }
};

template <typename T, size_t N, bool Flat>
bool DataInport<T, N, Flat>::isConnected() const {
    if constexpr (N == 0) {
        return !outports_.empty();
    } else {
        return outports_.size() >= 1 && outports_.size() <= N;
    }
}

template <typename T, size_t N, bool Flat>
bool DataInport<T, N, Flat>::hasData() const {
    if constexpr (N == 0) {
        return isConnected() && this->begin() != this->end();
    } else {
        return isConnected() &&
               util::all_of(outports_, [](auto* p) { return p->port()->hasData(); });
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

    for (auto* outport : outports_) {
        // Safe to static cast since we are unable to connect other outport types.
        if (Flat) {
            res.append_range(std::views::iota(0uz, outport->size()) |
                             std::views::transform([&](size_t i) {
                                 return std::pair{outport->port(), outport->getElement(i)};
                             }));
        } else {
            if (auto data = outport->getElement(0)) {
                res.emplace_back(outport->port(), data);
            }
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

    Document doc = getDefaultPortInfo(this, name.view());

    if (hasData()) {
        doc.append("p").append(DataTraits<T>::info(*getData()));
    } else {
        doc.append("p", "Port has no data");
    }
    return doc;
}

template <typename T, size_t N, bool Flat>
DataInfo DataInport<T, N, Flat>::getDataInfo() const {
    return util::dataInfoFor<T>();
}

}  // namespace inviwo
