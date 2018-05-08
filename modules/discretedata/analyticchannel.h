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
#include <inviwo/core/util/assertion.h>

#include "datachannel.h"

namespace inviwo {
/** \class DataAnalytic
    \brief Data channel by function evaluated at each (linear) index.

    Realization of DataChannel.

    Data is stored implicitly by a function f:index -> vec<T>,
    where the destination memory is pre-allocated.
    Indices are linear.

    @author Anke Friederici and Tino Weinkauf
*/
template <typename T>
class AnalyticChannel : public DataChannel<T> {
    // Types
public:
    typedef typename std::function<void(T*, ind)> Function;

    // Construction / Deconstruction
public:
    /** \brief Direct construction
    *
    *   @param dataFunction Data generator, mapping of linear index to T*
    *   @param numElements Total number of indexed positions
    *   @param numComponents Size of vector at each position
    *   @param name Name associated with the channel
    *   @param definedOn GridPrimitive the data is defined on, default: 0D vertices
    */
    AnalyticChannel(Function dataFunction, ind numElements, ind numComponents,
                    const std::string& name, GridPrimitive definedOn = GridPrimitive::Vertex)
        : DataChannel<T>(numComponents, name, definedOn)
        , numElements_(numElements)
        , dataFunction_(dataFunction) {}

    virtual ~AnalyticChannel() = default;

    // Methods
public:
    ind getNumElements() const override { return numElements_; }

    /** \brief Indexed point access, constant
    *   @param dest Position to write to, expect write of NumComponents many T
    *   @param index Linear point index
    */
    void fill(T* dest, ind index) const override;

    // Attributes
public:
    ind numElements_;
    Function dataFunction_;
};

/*--------------------------------------------------------------*
*  Implementations                                              *
*--------------------------------------------------------------*/

template <typename T>
void AnalyticChannel<T>::fill(T* const dest, const ind index) const {
    ivwAssert(index >= 0 && index < numElements_, "Index out of bounds: " << index);

    dataFunction_(dest, index);
}

/*--------------------------------------------------------------*
*  Exported Types                                               *
*--------------------------------------------------------------*/

typedef AnalyticChannel<float>      AnalyticChannelFloat;
typedef AnalyticChannel<double>     AnalyticChannelDouble;
typedef AnalyticChannel<char>       AnalyticChannelChar;
typedef AnalyticChannel<short>      AnalyticChannelShort;
typedef AnalyticChannel<int>        AnalyticChannelInt;
typedef AnalyticChannel<long>       AnalyticChannelLong;
typedef AnalyticChannel<long long>  AnalyticChannelLongLong;

typedef AnalyticChannel<unsigned char>      AnalyticChannelUChar;
typedef AnalyticChannel<unsigned short>     AnalyticChannelUShort;
typedef AnalyticChannel<unsigned int>       AnalyticChannelUInt;
typedef AnalyticChannel<unsigned long>      AnalyticChannelULong;
typedef AnalyticChannel<unsigned long long> AnalyticChannelULongLong;

}  // namespace
