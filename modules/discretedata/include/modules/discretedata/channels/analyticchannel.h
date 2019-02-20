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

#include <modules/discretedata/channels/datachannel.h>
#include <modules/discretedata/channels/channelgetter.h>
#include <modules/discretedata/channels/cachedgetter.h>

namespace inviwo {
namespace discretedata {

/**
 * \brief Data channel by function evaluated at each (linear) index.
 *
 * Realization of DataChannel.
 *
 * Data is stored implicitly by a function f:index -> vec<T, N>,
 * where the destination memory is pre-allocated.
 * Indices are linear.
 *
 *   @author Anke Friederici and Tino Weinkauf
 */
template <typename T, ind N, typename Vec = std::array<T, N>>
class AnalyticChannel : public DataChannel<T, N> {
public:
    static_assert(sizeof(Vec) == sizeof(T) * N, "Size and type do not agree with the vector type.");
    using Function = typename std::function<void(Vec&, ind)>;

public:
    /**
     * \brief Direct construction
     * @param dataFunction Data generator, mapping of linear index to T*
     * @param numElements Total number of indexed positions
     * @param name Name associated with the channel
     * @param definedOn GridPrimitive the data is defined on, default: 0D vertices
     */
    AnalyticChannel(Function dataFunction, ind numElements, const std::string& name,
                    GridPrimitive definedOn = GridPrimitive::Vertex)
        : DataChannel<T, N>(name, definedOn)
        , numElements_(numElements)
        , dataFunction_(dataFunction) {}

    virtual ~AnalyticChannel() = default;

public:
    ind size() const override { return numElements_; }

    /**
     * \brief Indexed point access, constant
     * Will write to the memory of dest via reinterpret_cast.
     * @param dest Position to write to, expect write of NumComponents many T
     * @param index Linear point index
     */
    void fillRaw(T* dest, ind index) const override {
        Vec& destVec = *reinterpret_cast<Vec*>(dest);
        dataFunction_(destVec, index);
    }

protected:
    virtual CachedGetter<AnalyticChannel>* newIterator() override {
        return new CachedGetter<AnalyticChannel>(this);
    }

public:
    ind numElements_;
    Function dataFunction_;
};

}  // namespace discretedata
}  // namespace inviwo
