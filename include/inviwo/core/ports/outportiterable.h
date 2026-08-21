/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2026 Inviwo Foundation
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
#include <inviwo/core/datastructures/datasequence.h>
#include <inviwo/core/util/typelist.h>

#include <vector>
#include <memory>

namespace inviwo {

class Outport;

template <typename T>
class DataOutport;

template <typename T>
struct DataOutportInterface {
    DataOutportInterface() = default;
    DataOutportInterface(const DataOutportInterface&) = delete;
    DataOutportInterface(DataOutportInterface&&) = delete;
    DataOutportInterface& operator=(const DataOutportInterface&) = delete;
    DataOutportInterface& operator=(DataOutportInterface&&) = delete;
    virtual ~DataOutportInterface() = default;
    virtual size_t size() const = 0;
    virtual std::shared_ptr<const T> getElement(size_t i) const = 0;
    virtual bool flat() const = 0;
    virtual Outport* port() = 0;
};

namespace detail {

template <typename T>
struct DataOutportFlat;

template <typename T>
struct DataOutportImpl;

template <typename Self, typename T, bool Flat>
struct DataOutportBase : DataOutportInterface<T> {
    virtual size_t size() const final {
        if constexpr (Flat && requires {
                          { getElements()->size() } -> std::convertible_to<size_t>;
                      }) {
            if (auto data = getElements()) {
                return data->size();
            } else {
                return 0;
            }
        } else {
            return 1;
        }
    }
    auto getElements() const { return static_cast<const Self*>(this)->getData(); }
    virtual bool flat() const final { return Flat; }
    virtual Outport* port() final { return static_cast<Self*>(this); }
};

template <typename Self, typename T>
struct DataOutportRegular : DataOutportBase<Self, T, false> {
    virtual std::shared_ptr<const T> getElement(size_t i) const final {
        if (auto data = this->getElements()) {
            if (i == 0) {
                return data;
            }
        }
        return nullptr;
    }
};

// Specialization for vector of data
template <typename T, typename Alloc>
struct DataOutportFlat<DataOutport<std::vector<T, Alloc>>>
    : DataOutportBase<DataOutport<std::vector<T, Alloc>>, T, true> {
    virtual std::shared_ptr<const T> getElement(size_t i) const final {
        if (auto data = this->getElements()) {
            if (i < data->size()) {
                return std::shared_ptr<const T>(data, &(*data)[i]);
            }
        }
        return nullptr;
    }
};

// Specialization for vector of data pointer
template <typename T, typename Alloc>
struct DataOutportFlat<DataOutport<std::vector<T*, Alloc>>>
    : DataOutportBase<DataOutport<std::vector<T*, Alloc>>, T, true> {
    virtual std::shared_ptr<const T> getElement(size_t i) const final {
        if (auto data = this->getElements()) {
            if (i < data->size()) {
                return std::shared_ptr<const T>(data, (*data)[i]);
            }
        }
        return nullptr;
    }
};

// Specialization for vector of data unique pointer
template <typename T, typename Alloc>
struct DataOutportFlat<DataOutport<std::vector<std::unique_ptr<T>, Alloc>>>
    : DataOutportBase<DataOutport<std::vector<std::unique_ptr<T>, Alloc>>, T, true> {
    virtual std::shared_ptr<const T> getElement(size_t i) const final {
        if (auto data = this->getElements()) {
            if (i < data->size()) {
                return std::shared_ptr<const T>(data, (*data)[i].get());
            }
        }
        return nullptr;
    }
};
// Specialization for DataSequence of data
template <typename T>
struct DataOutportFlat<DataOutport<DataSequence<T>>>
    : DataOutportBase<DataOutport<DataSequence<T>>, T, true> {
    virtual std::shared_ptr<const T> getElement(size_t i) const final {
        if (auto data = this->getElements()) {
            if (i < data->size()) {
                return (*data)[i];
            }
        }
        return nullptr;
    }
};

template <typename T>
concept hasBases = requires { typename T::Bases; };

template <typename T>
struct Bases {
    using type = TypeList<T>;
};

template <typename... Ts>
struct Bases<TypeList<Ts...>> {
    using type = JoinTypeLists<typename Bases<Ts>::type...>;
};

template <hasBases T>
struct Bases<T> {
    using type = JoinTypeLists<TypeList<T>, typename Bases<typename T::Bases>::type>;
};

template <typename T>
using bases_t = Bases<T>::type;

template <typename Self, typename T>
struct DataBases {};
template <typename Self, template <typename...> typename L, typename... Ts>
struct DataBases<Self, L<Ts...>> : DataOutportRegular<Self, Ts>... {};

template <typename T>
struct DataOutportImpl<DataOutport<T>> : DataBases<DataOutport<T>, bases_t<T>> {};

template <typename T, typename Alloc>
struct DataOutportImpl<DataOutport<std::vector<T, Alloc>>>
    : DataOutportRegular<DataOutport<std::vector<T, Alloc>>, std::vector<T, Alloc>>,
      DataOutportFlat<DataOutport<std::vector<T, Alloc>>> {};

template <typename T, typename Alloc>
struct DataOutportImpl<DataOutport<std::vector<T*, Alloc>>>
    : DataOutportRegular<DataOutport<std::vector<T*, Alloc>>, std::vector<T*, Alloc>>,
      DataOutportFlat<DataOutport<std::vector<T*, Alloc>>> {};

template <typename T, typename Alloc>
struct DataOutportImpl<DataOutport<std::vector<std::unique_ptr<T>, Alloc>>>
    : DataOutportRegular<DataOutport<std::vector<std::unique_ptr<T>, Alloc>>,
                         std::vector<std::unique_ptr<T>, Alloc>>,
      DataOutportFlat<DataOutport<std::vector<std::unique_ptr<T>, Alloc>>> {};

template <typename T>
struct DataOutportImpl<DataOutport<DataSequence<T>>>
    : DataOutportRegular<DataOutport<DataSequence<T>>, DataSequence<T>>,
      DataOutportFlat<DataOutport<DataSequence<T>>> {};

}  // namespace detail

}  // namespace inviwo
