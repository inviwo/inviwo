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
#include <modules/discretedata/discretedatamoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/metadata/metadataowner.h>

namespace inviwo {
namespace discretedata {

/** Discretedata index type **/
using ind = signed long long;

/** Mapping structure name to respective dimension.
 *  Assign channels to any dimensions this way.
 *  If these do not suffice, cast the respective short.
 */
enum class GridPrimitive : ind {
    Undef = -1,
    Vertex = 0,
    Edge = 1,
    Face = 2,
    Volume = 3,
    HyperVolume = 4
};

template <typename VecNT, typename T, ind N>
class ChannelIterator;

template <typename VecNT, typename T, ind N>
class ConstChannelIterator;

template <typename T, ind N>
struct ChannelGetter;

template <typename T, ind N>
class DataChannel;

/** \class Channel
    \brief An untyped scalar or vector component of a data set.

    General version of a DataChannel for use in general containers
    (see DataSet).

    @author Anke Friederici and Tino Weinkauf
*/
class IVW_MODULE_DISCRETEDATA_API Channel : public MetaDataOwner {
    // Construction / Deconstruction
public:
    /** \brief Direct construction
     *   @param numComponents Size of vector at each position
     *   @param name Name associated with the channel
     *   @param definedOn GridPrimitive the data is defined on, default: 0D vertices
     */
    Channel(ind numComponents, const std::string& name, DataFormatId dataFormat,
            GridPrimitive definedOn = GridPrimitive::Vertex);

    virtual ~Channel() = default;

public:
    /** Returns the "Name" meta data */
    const std::string getName() const;

    /** Sets the "Name" meta data */
    void setName(const std::string&);

    /** Returns the "GridPrimitiveType" meta data */
    GridPrimitive getGridPrimitiveType() const;

    /** Returns the "DataFormatId" meta data */
    DataFormatId getDataFormatId() const;

    /** Returns the "NumComponents" meta data */
    ind getNumComponents() const;

protected:
    /** Sets the "GridPrimitiveType" meta data
     *   Should be constant, only DataSet is allowed to write.
     */
    void setGridPrimitiveType(GridPrimitive);

    /** Sets the "DataFromatId" meta data
     */
    void setDataFormatId(DataFormatId);

    /** Sets the "NumComponents" meta data
     *   Should be constant, only DataSet is allowed to write.
     */
    void setNumComponents(ind);

public:
    virtual ind size() const = 0;
};

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

template <typename T, ind N>
class VectorChannel : public BaseChannel<T, N> {

protected:
    VectorChannel(const std::string& name, DataFormatId dataFormat,
                  GridPrimitive definedOn = GridPrimitive::Vertex)
        : BaseChannel<T, N>(name, dataFormat, definedOn) {}

public:
    template <typename VecNT>
    ChannelIterator<VecNT, T, N> begin() {
        return ChannelIterator<VecNT, T, N>(this->newIterator(), 0);
    }
    template <typename VecNT>
    ChannelIterator<VecNT, T, N> end() {
        return ChannelIterator<VecNT, T, N>(this->newIterator(), this->size());
    }

    template <typename VecNT>
    ConstChannelIterator<VecNT, T, N> begin() const {
        return ConstChannelIterator<VecNT, T, N>((DataChannel<T, N>*)this, 0);
    }
    template <typename VecNT>
    ConstChannelIterator<VecNT, T, N> end() const {
        return ConstChannelIterator<VecNT, T, N>((DataChannel<T, N>*)this, this->size());
    }
};

template <typename T>
class ScalarChannel : public BaseChannel<T, 1> {

public:
    template <typename Vec1T = T>
    using iterator = ChannelIterator<Vec1T, T, 1>;
    template <typename Vec1T = T>
    using const_iterator = ConstChannelIterator<Vec1T, T, 1>;

    // Methods

    ScalarChannel(const std::string& name, DataFormatId dataFormat,
                  GridPrimitive definedOn = GridPrimitive::Vertex)
        : BaseChannel<T, 1>(name, dataFormat, definedOn) {}

    void operator()(T& dest, int index) const { this->fillRaw(&dest, index); }

    template <typename Vec1T = T>
    iterator<Vec1T> begin() {
        return iterator<Vec1T>(this->newIterator(), 0);
    }
    template <typename Vec1T = T>
    iterator<Vec1T> end() {
        return iterator<Vec1T>(this->newIterator(), this->size());
    }

    template <typename Vec1T = T>
    const_iterator<Vec1T> begin() const {
        return const_iterator<Vec1T>(static_cast<const DataChannel<T, 1>*>(this), 0);
    }
    template <typename Vec1T = T>
    const_iterator<Vec1T> end() const {
        return const_iterator<Vec1T>(static_cast<const DataChannel<T, 1>*>(this), this->size());
    }
};

#define BaseChannelDef std::conditional<N == 1, ScalarChannel<T>, VectorChannel<T, N>>::type

/** \class DataChannel
    \brief A single vector component of a data set.

    The type is arbitrary but is expected to support
    the basic arithmetic operations.
    It is specified via type, base type and number of components.

    Several realizations extend this pure virtual class
    that differ in data storage/generation.
    Direct indexing is virtual, avoid where possible.

    @author Anke Friederici and Tino Weinkauf
*/
template <typename T, ind N>
class DataChannel : public BaseChannelDef {

    using BaseChannel = typename BaseChannelDef;

#undef BaseChannelDef

    friend class DataSet;
    friend struct ChannelGetter<T, N>;

public:
    template <typename VecNT>
    using iterator = ChannelIterator<VecNT, T, N>;

    template <typename VecNT>
    using const_iterator = ConstChannelIterator<VecNT, T, N>;

private:
    using MetaScalarType = MetaDataPrimitiveType<double, N, 0>;
    using MetaVec = typename inviwo::util::glmtype<double, N, 1>::type;
    using GlmVector = typename inviwo::util::glmtype<T, N, 1>::type;

    // Construction / Deconstruction
public:
    /** \brief Direct construction
     *   @param name Name associated with the channel
     *   @param definedOn GridPrimitive the data is defined on, default: 0D vertices
     */
    DataChannel(const std::string& name, GridPrimitive definedOn = GridPrimitive::Vertex);

    virtual ~DataChannel() = default;

public:
    /** \brief Indexed point access, copy data
     *   Thread safe.
     *   @param dest Position to write to, expect T[NumComponents]
     *   @param index Linear point index
     */
    template <typename VecNT>
    void fill(VecNT& dest, ind index) const {
        static_assert(sizeof(VecNT) == sizeof(T) * N,
                      "Size and type do not agree with the vector type.");
        this->fillRaw(reinterpret_cast<T*>(&dest), index);
    }

    template <typename VecNT>
    struct ChannelRange {
        static_assert(sizeof(VecNT) == sizeof(T) * N,
                      "Size and type do not agree with the vector type.");
        typedef ChannelIterator<VecNT, T, N> iterator;

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
        using const_iterator = ConstChannelIterator<VecNT, T, N>;

        ConstChannelRange(const DataChannel<T, N>* channel) : parent_(channel) {}

        const_iterator begin() const { return parent_->template begin<VecNT>(); }
        const_iterator end() const { return parent_->template end<VecNT>(); }

    private:
        const DataChannel<T, N>* parent_;
    };

    /** \brief Get iterator range
     *   Templated iterator return type, only specified once.
     *   @tparam VecNT Return type of resulting iterators
     */
    template <typename VecNT>
    ChannelRange<VecNT> all() {
        return ChannelRange<VecNT>(this);
    }

    /** \brief Get const iterator range
     *   Templated iterator return type, only specified once.
     *   @tparam VecNT Return type of resulting iterators
     */
    template <typename VecNT>
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
};

}  // namespace discretedata
}  // namespace inviwo

// Circumvent circular reference.
#include "channeliterator.h"

#include "datachannel.inl"
