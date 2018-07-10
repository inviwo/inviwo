/*********************************************************************************
*
* Inviwo - Interactive Visualization Workshop
*
* Copyright (c) 2012-2018 Inviwo Foundation
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

#include <discretedata/discretedatamoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include "datachannel.h"
#include "channelgetter.h"

namespace inviwo {
namespace dd {

/** \class DataBuffer
    \brief Data channel as array data

    Data block with a size of
    NumDataPoints * NumComponents.
    The buffer is not constant, copy to change.

    @author Anke Friederici and Tino Weinkauf
*/
template <typename T, ind N>
class BufferChannel : public DataChannel<T, N> {

    friend class DataSet;

    // Construction / Deconstruction
public:
    /** \brief Direct construction, empty data
    *   @param numElements Total number of indexed positions
    *   @param numComponents Size of vector at each position
    *   @param name Name associated with the channel
    *   @param definedOn GridPrimitive the data is defined on, default: 0D vertices
    */
    BufferChannel(ind numElements, const std::string& name,
                  GridPrimitive definedOn = GridPrimitive::Vertex)
        : DataChannel<T, N>(name, definedOn), buffer_(numElements * N) {}

    /** \brief Direct construction
    *   @param data Raw data, copy values
    *   @param name Name associated with the channel
    *   @param definedOn GridPrimitive the data is defined on, default: 0D vertices
    */
    BufferChannel(const std::vector<T>& rawData, const std::string& name,
                  GridPrimitive definedOn = GridPrimitive::Vertex)
        : DataChannel<T, N>(name, definedOn)
        , buffer_(rawData) {}

    /** \brief Direct construction
    *   @param data Raw data, move values
    *   @param name Name associated with the channel
    *   @param definedOn GridPrimitive the data is defined on, default: 0D vertices
    */
    BufferChannel(std::vector<T>&& data, const std::string& name,
                  GridPrimitive definedOn = GridPrimitive::Vertex)
        : DataChannel<T, N>(name, definedOn)
        , buffer_(std::move(data)) {}

    /** \brief Direct construction
    *   @param data Pointer to data, copy numElements * numComponents
    *   @param numElements Total number of indexed positions
    *   @param name Name associated with the channel
    *   @param definedOn GridPrimitive the data is defined on, default: 0D vertices
    */
    BufferChannel(T* const data, ind numElements, const std::string& name,
                  GridPrimitive definedOn = GridPrimitive::Vertex)
        : DataChannel<T, N>(name, definedOn)
        , buffer_(data, data + numElements * N) {}

protected:
    virtual ChannelGetter<T, N>* newIterator() { return reinterpret_cast<ChannelGetter<T, N>*>(new BufferGetter<T, N>(this)); }

    /** \brief Indexed point access, constant
    *   @param dest Position to write to, expect write of NumComponents many T
    *   @param index Linear point index
    */
    virtual void fillRaw(T* dest, ind index) const override { memcpy(dest, &buffer_[index * N], sizeof(T) * N); }

    // Methods
public:
    virtual ind size() const override { return buffer_.size() / N; }

    /** \brief Indexed point access, mutable, same as []
    *   NOT THREAD SAFE, use fill instead.
    *   @param index Linear point index
    *   @return Reference to data
    */
    template<typename VecNT>
    VecNT& get(ind index) {
        static_assert(sizeof(VecNT) == sizeof(T) * N, "Size and type do not agree with the vector type."); 
        return *reinterpret_cast<VecNT*>(&buffer_[index*N]);
    }

    // Attributes
protected:
    /** \brief Vector containing the buffer data
    *   Resizeable only by DataSet. Handle with care:
    *   Resize invalidates pointers to memory, but iterators remain valid.
    */
    std::vector<T> buffer_;
};

}  // namespace
}