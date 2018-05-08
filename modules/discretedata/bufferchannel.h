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

namespace inviwo {

/** \class DataBuffer
    \brief Data channel as array data

    Data block with a size of
    NumDataPoints * NumComponents.
    The buffer is not constant, copy to change.

    @author Anke Friederici and Tino Weinkauf
*/
template <typename T>
class BufferChannel : public DataChannel<T> {

    friend class DataSet;

    // Construction / Deconstruction
public:
    /** \brief Direct construction, empty data
    *   @param numElements Total number of indexed positions
    *   @param numComponents Size of vector at each position
    *   @param name Name associated with the channel
    *   @param definedOn GridPrimitive the data is defined on, default: 0D vertices
    */
    BufferChannel(ind numElements, ind numComponents, const std::string& name,
                  GridPrimitive definedOn = GridPrimitive::Vertex)
        : DataChannel<T>(numComponents, name, definedOn), buffer_(numElements * numComponents) {}

    /** \brief Direct construction
    *   @param data Raw data, copy values
    *   @param numComponents Size of vector at each position
    *   @param name Name associated with the channel
    *   @param definedOn GridPrimitive the data is defined on, default: 0D vertices
    */
    BufferChannel(std::vector<T>& data, ind numComponents, const std::string& name,
                  GridPrimitive definedOn = GridPrimitive::Vertex)
        : DataChannel<T>(numComponents, name, definedOn), buffer_(data) {
        ivwAssert(data.size() % numComponents == 0, "Data size not multiple of numComponents.");
    }

    /** \brief Direct construction
    *   @param data Raw data, move values
    *   @param numComponents Size of vector at each position
    *   @param name Name associated with the channel
    *   @param definedOn GridPrimitive the data is defined on, default: 0D vertices
    */
    BufferChannel(std::vector<T>&& data, ind numComponents, const std::string& name,
                  GridPrimitive definedOn = GridPrimitive::Vertex)
        : DataChannel<T>(numComponents, name, definedOn), buffer_(std::move(data)) {
        ivwAssert(data.size() % numComponents == 0, "Data size not multiple of numComponents.");
    }

    /** \brief Direct construction
    *   @param data Pointer to data, copy numElements * numComponents
    *   @param numElements Total number of indexed positions
    *   @param numComponents Size of vector at each position
    *   @param name Name associated with the channel
    *   @param definedOn GridPrimitive the data is defined on, default: 0D vertices
    */
    BufferChannel(T* data, ind numElements, ind numComponents, const std::string& name,
                  GridPrimitive definedOn = GridPrimitive::Vertex)
        : DataChannel<T>(numComponents, name, definedOn)
        , buffer_(data, data + numElements * numComponents) {}

protected:

    // Methods
public:
    virtual ind getNumElements() const override;

    /** \brief Indexed point access, constant
    *   @param index Linear point index
    *   @return Pointer to data, size T[NumComponents]
    */
    const T* operator[](ind index) const { return get(index); }

    /** \brief Indexed point access, mutable
    *   @param index Linear point index
    *   @return Pointer to data, size T[NumComponents]
    */
    T* operator[](ind index) { return get(index); }

    /** \brief Indexed point access, mutable, same as []
    *   NOT THREAD SAFE, use fill instead.
    *   @param index Linear point index
    *   @return Pointer to data, NumComponents many T
    */
    T* get(ind index);

    /** \brief Indexed point access, constant, same as []
    *   @param index Linear point index
    *   @return Pointer to data, NumComponents many T
    */
    const T* get(ind index) const;

    /** \brief Indexed point access, constant
    *   @param dest Position to write to, expect write of NumComponents many T
    *   @param index Linear point index
    */
    void fill(T* dest, ind index) const override;

    // Attributes
protected:
    /** \brief Vector containing the buffer data
    *   Resizeable only by DataSet. Handle with care:
    *   Resize invalidates pointers to memory, but iterators remain valid.
    */
    std::vector<T> buffer_;
};

/*--------------------------------------------------------------*
*  Implementations - Index Access                               *
*--------------------------------------------------------------*/

template <typename T>
ind BufferChannel<T>::getNumElements() const {
    ivwAssert(buffer_.size() % NumComponents == 0, "Buffer size not multiple of component size");

    return buffer_.size() / NumComponents;
}

template <typename T>
T* BufferChannel<T>::get(const ind index) {
    ivwAssert(index >= 0 && index < getNumElements(), "Index out of bounds: " << index);

    return buffer_.data() + index * NumComponents;
}

template <typename T>
const T* BufferChannel<T>::get(const ind index) const {
    ivwAssert(index >= 0 && index < getNumElements(), "Index out of bounds: " << index);

    return buffer_.data() + index * NumComponents;
}

template <typename T>
void BufferChannel<T>::fill(T* const dest, const ind index) const {
    ivwAssert(index >= 0 && index < getNumElements(), "Index out of bounds: " << index);

    memcpy(dest, buffer_.data() + (index * NumComponents), sizeof(T) * NumComponents);
}

/*--------------------------------------------------------------*
*  Exported Types                                               *
*--------------------------------------------------------------*/

 typedef BufferChannel<float>      BufferChannelFloat;
 typedef BufferChannel<double>     BufferChannelDouble;
 typedef BufferChannel<char>       BufferChannelChar;
 typedef BufferChannel<short>      BufferChannelShort;
 typedef BufferChannel<int>        BufferChannelInt;
 typedef BufferChannel<long>       BufferChannelLong;
 typedef BufferChannel<long long>  BufferChannelLongLong;

 typedef BufferChannel<unsigned char>       BufferChannelUChar;
 typedef BufferChannel<unsigned short>      BufferChannelUShort;
 typedef BufferChannel<unsigned int>        BufferChannelUInt;
 typedef BufferChannel<unsigned long>       BufferChannelULong;
 typedef BufferChannel<unsigned long long>  BufferChannelULongLong;

}  // namespace
