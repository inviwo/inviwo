/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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

#include <modules/discretedatapython/discretedatapythonmoduledefine.h>
// push/pop warning state to prevent disabling some warnings by pybind headers
#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <warn/pop>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/assertion.h>

#include <modules/discretedata/channels/datachannel.h>
#include <modules/discretedata/channels/channelgetter.h>
#include <modules/discretedata/channels/buffergetter.h>

#include <modules/python3/pybindutils.h>

namespace inviwo {
namespace discretedata {

/**
 * \brief Data channel from numpy array (via pybind11)
 *
 * Data block with a size of
 * NumDataPoints * NumComponents.
 * The buffer is shared with the python code to circumvent deep copying.
 */
template <typename T, ind N = 1>
class IVW_MODULE_DISCRETEDATAPYTHON_API NumPyChannel : public DataChannel<T, N> {
    friend class DataSet;
    friend struct BufferGetter<NumPyChannel>;
    using DefaultVec = typename DataChannel<T, N>::DefaultVec;

private:
    void* getRaw(ind index) {
        return reinterpret_cast<char*>(buffer_->mutable_data()) + buffer_->itemsize() * index;
    }
    const void* getRaw(ind index) const {
        return reinterpret_cast<const char*>(buffer_->data()) + buffer_->itemsize() * index;
    }

public:
    /**
     * \brief Direct construction, empty data
     * @param numElements Total number of indexed positions
     * @param name Name associated with the channel
     * @param definedOn GridPrimitive the data is defined on, default: vertices (0D)
     */
    NumPyChannel(ind numElements, const std::string& name,
                 GridPrimitive definedOn = GridPrimitive::Vertex)
        : DataChannel<T, N>(name, definedOn), buffer_(numElements * N) {}

    /**
     * \brief Direct construction
     * @param rawData Raw data, copy values
     * @param name Name associated with the channel
     * @param definedOn GridPrimitive the data is defined on, default: 0D vertices
     */
    NumPyChannel(const pybind11::array& rawData, const std::string& name,
                 GridPrimitive definedOn = GridPrimitive::Vertex)
        : DataChannel<T, N>(name, definedOn), buffer_(std::make_shared<pybind11::array>(rawData)) {}

    /**
     * \brief Direct construction
     * @param data Pointer to data
     * @param shape DImensions of data
     * @param name Name associated with the channel
     * @param definedOn GridPrimitive the data is defined on, default: 0D vertices
     */
    NumPyChannel(const pybind11::dtype& data, pybind11::array::ShapeContainer shape,
                 const std::string& name,  // const void* ptr = nullptr,
                 GridPrimitive definedOn = GridPrimitive::Vertex)
        : DataChannel<T, N>(name, definedOn), buffer_(data, std::move(shape)) {}

    /**
     * \brief Copy construction from any kind of DataChannel
     * @param channel DataChannel to copy from
     */
    NumPyChannel(const DataChannel<T, N>& channel)
        : DataChannel<T, N>(channel.getName(), channel.getGridPrimitiveType())
        , buffer_(channel.size()) {
        channel.fillRaw(buffer_->data(), 0, channel.size());
    }

    virtual Channel* clone() const override { return new NumPyChannel<T, N>(*this); }

    virtual ind size() const override { return buffer_->size(); }

    const std::shared_ptr<pybind11::array> data() const { return buffer_; }
    std::shared_ptr<pybind11::array> data() { return buffer_; }

    /**
     * \brief Indexed point access
     * @param index Linear point index
     * @return Reference to data
     */
    DefaultVec& operator[](ind index) { return *reinterpret_cast<DefaultVec*>(getRaw(index)); }

    /**
     * \brief Indexed point access
     * @param index Linear point index
     * @return Reference to data
     */
    const DefaultVec& operator[](ind index) const {
        return *reinterpret_cast<const DefaultVec*>(buffer_->data() + buffer_->itemsize() * index);
    }

    /**
     * \brief Indexed point access
     * @param index Linear point index
     * @return Reference to data
     */
    template <typename VecNT = DefaultVec>
    VecNT& get(ind index) {
        static_assert(sizeof(VecNT) == sizeof(T) * N,
                      "Size and type do not agree with the vector type.");
        return *reinterpret_cast<VecNT*>(getRaw(index));
    }

    /**
     * \brief Indexed point access
     * @param index Linear point index
     * @return Reference to data
     */
    template <typename VecNT = DefaultVec>
    const VecNT& get(ind index) const {
        static_assert(sizeof(VecNT) == sizeof(T) * N,
                      "Size and type do not agree with the vector type.");
        return *reinterpret_cast<VecNT*>(getRaw(index));
    }

protected:
    virtual BufferGetter<NumPyChannel<T, N>>* newIterator() override {
        return new BufferGetter<NumPyChannel<T, N>>(this);
    }

    /**
     * \brief Indexed point access, constant
     * @param dest Position to write to, expect write of NumComponents many T
     * @param index Linear point index
     */
    virtual void fillRaw(T* dest, ind index, ind numElements) const override {
        memcpy(dest, getRaw(index), sizeof(T) * N * numElements);
    }

private:
    /**
     * \brief Numpy array containing the buffer data
     */
    std::shared_ptr<pybind11::array> buffer_;
};
}  // namespace discretedata
}  // namespace inviwo
