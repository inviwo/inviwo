/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <modules/discretedata/discretedatamoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/discretedata/discretedatatypes.h>
#include <modules/discretedata/channels/channel.h>
#include <modules/discretedata/channels/channelgetter.h>
#include <modules/discretedata/channels/channeliterator.h>

namespace inviwo {
namespace discretedata {

template <typename T, ind N>
class BaseChannel : public Channel {
public:
    BaseChannel(const std::string& name, DataFormatId dataFormat,
                GridPrimitive definedOn = GridPrimitive::Vertex)
        : Channel(N, name, dataFormat, definedOn) {}

protected:
    virtual void fillRaw(T* dest, ind index) const = 0;
    virtual ChannelGetter<T, N>* newIterator() = 0;
};

/**
 * \brief A single vector component of a data set.
 *
 * The type is arbitrary but is expected to support the basic arithmetic operations.
 * It is specified via type, base type and number of components.
 *
 * Several realizations extend this pure virtual class that differ in data storage/generation.
 * Direct indexing is virtual, avoid where possible.
 *
 * @author Anke Friederici and Tino Weinkauf
 */
template <typename T, ind N>
class DataChannel : public BaseChannel<T, N> {
    friend class DataSet;
    friend struct ChannelGetter<T, N>;

public:
    static constexpr ind num_comp = N;
    using value_type = T;

    template <typename VecNT>
    using iterator = ChannelIterator<VecNT, T, N>;
    template <typename VecNT = T>
    using const_iterator = ConstChannelIterator<DataChannel<T, N>, VecNT>;

    using DefaultVec =
        typename std::conditional<(N <= 4), typename inviwo::util::glmtype<T, N>::type,
                                  std::array<T, N>>::type;

public:
    /**
     * \brief Direct construction
     * @param name Name associated with the channel
     * @param definedOn GridPrimitive the data is defined on, default: 0D vertices
     */
    DataChannel(const std::string& name, GridPrimitive definedOn = GridPrimitive::Vertex);
    virtual ~DataChannel() = default;

    /**
     * \brief Indexed point access, copy data
     * Thread safe.
     * @param dest Position to write to, expect T[NumComponents]
     * @param index Linear point index
     */
    template <typename VecNT>
    void fill(VecNT& dest, ind index) const {
        static_assert(sizeof(VecNT) == sizeof(T) * N,
                      "Size and type do not agree with the vector type.");
        this->fillRaw(reinterpret_cast<T*>(&dest), index);
    }

    template <typename VecNT>
    void operator()(VecNT& dest, ind index) const {
        fill(dest, index);
    }

    template <typename VecNT = DefaultVec>
    iterator<VecNT> begin() {
        return iterator<VecNT>(this->newIterator(), 0);
    }
    template <typename VecNT = DefaultVec>
    iterator<VecNT> end() {
        return iterator<VecNT>(this->newIterator(), this->size());
    }

    template <typename VecNT = DefaultVec>
    const_iterator<VecNT> begin() const {
        return const_iterator<VecNT>(this, 0);
    }
    template <typename VecNT = DefaultVec>
    const_iterator<VecNT> end() const {
        return const_iterator<VecNT>(this, this->size());
    }

    template <typename VecNT>
    struct ChannelRange {
        static_assert(sizeof(VecNT) == sizeof(T) * N,
                      "Size and type do not agree with the vector type.");
        using iterator = DataChannel::iterator<VecNT>;

        ChannelRange(DataChannel<T, N>* channel) : parent_(channel) {}

        iterator begin() { return parent_->template begin<VecNT>(); }
        iterator end() { return parent_->template end<VecNT>(); }

    private:
        DataChannel<T, N>* parent_;
    };

    template <typename VecNT>
    struct ConstChannelRange {
        static_assert(sizeof(VecNT) == sizeof(T) * N,
                      "Size and type do not agree with the vector type.");
        using const_iterator = DataChannel::const_iterator<VecNT>;

        ConstChannelRange(const DataChannel<T, N>* channel) : parent_(channel) {}

        const_iterator begin() const { return parent_->template begin<VecNT>(); }
        const_iterator end() const { return parent_->template end<VecNT>(); }

    private:
        const DataChannel<T, N>* parent_;
    };

    /**
     * \brief Get iterator range
     * Templated iterator return type, only specified once.
     * @tparam VecNT Return type of resulting iterators
     */
    template <typename VecNT = DefaultVec>
    ChannelRange<VecNT> all() {
        return ChannelRange<VecNT>(this);
    }

    /**
     * \brief Get const iterator range
     * Templated iterator return type, only specified once.
     * @tparam VecNT Return type of resulting iterators
     */
    template <typename VecNT = DefaultVec>
    ConstChannelRange<VecNT> all() const {
        return ConstChannelRange<VecNT>(this);
    }

    template <typename VecNT>
    void getMin(VecNT& dest) const;

    template <typename VecNT>
    void getMax(VecNT& dest) const;

    template <typename VecNT>
    void getMinMax(VecNT& min, VecNT& max) const;

protected:
    void computeMinMax() const;

private:
    mutable std::array<double, N> min_;
    mutable std::array<double, N> max_;
    mutable bool validMinMax_ = false;
};

template <typename T, ind N>
DataChannel<T, N>::DataChannel(const std::string& name, GridPrimitive definedOn)
    : BaseChannel<T, N>(name, DataFormat<T>::id(), definedOn) {}

template <typename T, ind N>
template <typename VecNT>
void DataChannel<T, N>::getMin(VecNT& dest) const {
    static_assert(sizeof(VecNT) == sizeof(T) * N,
                  "Size and type do not agree with the vector type.");

    if (!validMinMax_) {
        computeMinMax();
    }

    T* rawVec = reinterpret_cast<T*>(&dest);
    for (ind i = 0; i < N; ++i) {
        rawVec[i] = min_[i];
    }
}

template <typename T, ind N>
template <typename VecNT>
void DataChannel<T, N>::getMax(VecNT& dest) const {
    static_assert(sizeof(VecNT) == sizeof(T) * N,
                  "Size and type do not agree with the vector type.");

    if (!validMinMax_) {
        computeMinMax();
    }

    T* rawVec = reinterpret_cast<T*>(&dest);
    for (ind i = 0; i < N; ++i) {
        rawVec[i] = max_[i];
    }
}

template <typename T, ind N>
template <typename VecNT>
void DataChannel<T, N>::getMinMax(VecNT& minDest, VecNT& maxDest) const {
    static_assert(sizeof(VecNT) == sizeof(T) * N,
                  "Size and type do not agree with the vector type.");
    getMin(minDest);
    getMax(maxDest);
}

template <typename T, ind N>
void DataChannel<T, N>::computeMinMax() const {
    using Vec = std::array<T, N>;

    Vec minT;
    Vec maxT;

    this->fill(minT, 0);
    this->fill(maxT, 0);

    for (const Vec& val : this->all<Vec>()) {
        for (ind dim = 0; dim < N; ++dim) {
            minT[dim] = std::min(minT[dim], val[dim]);
            maxT[dim] = std::max(maxT[dim], val[dim]);
        }
    }

    for (ind dim = 0; dim < N; ++dim) {
        min_[dim] = static_cast<double>(minT[dim]);
        max_[dim] = static_cast<double>(maxT[dim]);
    }

    validMinMax_ = true;
}

}  // namespace discretedata
}  // namespace inviwo
