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
#include <inviwo/core/metadata/metadataowner.h>

namespace inviwo {
namespace dd {

/** Discretedata index type **/
typedef signed long long ind;

/** Mapping structure name to respective dimension.
*   Assign channels to any dimensions this way.
*   If these do not suffice, cast the respective short.
*/
enum GridPrimitive : char {
    Vertex = 0,
    Edge = 1,
    Face = 2,
    Volume = 3,
    HyperVolume = 4
};

template<typename VecNT, typename T, ind N>
class ChannelIterator;

template<typename VecNT, typename T, ind N>
class ConstChannelIterator;

template<typename T, ind N>
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

    // Methods
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

    // Attributes
public:
    virtual ind size() const = 0;
};

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
class DataChannel : public Channel {

    friend class DataSet;
    friend struct ChannelGetter<T, N>;

    template <typename VecNT>
    using iterator = ChannelIterator<VecNT, T, N>;

    template <typename VecNT>
    using const_iterator = ConstChannelIterator<VecNT, T, N>;

    // Construction / Deconstruction
public:
    /** \brief Direct construction
    *   @param name Name associated with the channel
    *   @param definedOn GridPrimitive the data is defined on, default: 0D vertices
    */
    DataChannel(const std::string& name, GridPrimitive definedOn = GridPrimitive::Vertex);

    virtual ~DataChannel() = default;

    // Methods
protected:
    virtual void fillRaw(T* dest, ind index) const = 0;

    virtual ChannelGetter<T, N>* newIterator() = 0;

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
        fillRaw(reinterpret_cast<T*>(&dest), index);
    }

    template <typename VecNT>
    ChannelIterator<VecNT, T, N> begin() {
        return ChannelIterator<VecNT, T, N>(newIterator(), 0);
    }
    template <typename VecNT>
    ChannelIterator<VecNT, T, N> end() {
        return ChannelIterator<VecNT, T, N>(newIterator(), size());
    }

    template <typename VecNT>
    ConstChannelIterator<VecNT, T, N> begin() const {
        return ConstChannelIterator<VecNT, T, N>(this, 0);
    }
    template <typename VecNT>
    ConstChannelIterator<VecNT, T, N> end() const {
        return ConstChannelIterator<VecNT, T, N>(this, size());
    }

    template <typename VecNT>
    struct ChannelRange {
        static_assert(sizeof(VecNT) == sizeof(T) * N,
                      "Size and type do not agree with the vector type.");
        typedef ChannelIterator<VecNT, T, N> iterator;

        ChannelRange(DataChannel<T, N>* channel) : parent_(channel) {}

        iterator begin() { return parent_->template begin<VecNT>(); }
        iterator end()   { return parent_->template end<VecNT>(); }

    private:
        DataChannel<T, N>* parent_;
    };

    template <typename VecNT>
    struct ConstChannelRange {
        static_assert(sizeof(VecNT) == sizeof(T) * N,
                      "Size and type do not agree with the vector type.");
        typedef ConstChannelIterator<VecNT, T, N> const_iterator;

        ConstChannelRange(const DataChannel<T, N>* channel) : parent_(channel) {}

        const_iterator begin() const { return parent_->template begin<VecNT>(); }
        const_iterator end()   const { return parent_->template end<VecNT>(); }

    private:
        const DataChannel<T, N>* parent_;
    };

    /** \brief Get iterator range
    *   Templated iterator return type, only specified once.
    *   @tparam VecNT Return type of resulting iterators
    */
    template <typename VecNT>
    ChannelRange<VecNT> all() { return ChannelRange<VecNT>(this); }

    /** \brief Get const iterator range
    *   Templated iterator return type, only specified once.
    *   @tparam VecNT Return type of resulting iterators
    */
    template <typename VecNT>
    ConstChannelRange<VecNT> all() const { return ConstChannelRange<VecNT>(this); }
};

// Scalar Specialization
#define if_scalar template < typename sfinae  = typename std::enable_if<(N == 1)>::type, \
                             typename noparam = typename std::enable_if<std::is_same<sfinae, void>::value>::type >

    /** \brief Indexed scalar access, copy data
     *   Thread safe.
     *   @param dest Position to write to, expect T[NumComponents]
     *   @param index Linear point index
     */
    if_scalar void operator()(T& dest, ind index) const { fill(dest, index); }

    if_scalar iterator<T> begin() { return iterator<T>(newIterator(), 0); }
    if_scalar iterator<T> end()   { return iterator<T>(newIterator(), size()); }

    if_scalar const_iterator<T> begin() const { return const_iterator<T>(newIterator(), 0); }
    if_scalar const_iterator<T> end()   const { return const_iterator<T>(newIterator(), size()); }

#undef if_scalar
};

}  // namespace
}

// Circumvent circular reference.
#include "channeliterator.h"

#include "datachannel.inl"
