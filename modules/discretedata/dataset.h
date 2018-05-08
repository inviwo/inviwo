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
#include <inviwo/core/datastructures/datatraits.h>

#include "bufferchannel.h"
#include "connectivity.h"

namespace inviwo {
namespace dd {

typedef std::shared_ptr<Channel> SharedChannel;
typedef std::shared_ptr<const Channel> SharedConstChannel;

struct ChannelCompare {
    bool operator()(const std::pair<std::string, GridPrimitive>& u,
                    const std::pair<std::string, GridPrimitive>& v) const {
        return (u.second < v.second) || u.first.compare(v.first) < 0;
    }
};
// Map used for storing and querying channels by name and GridPrimitive type.
typedef std::map<
    std::pair<std::string, GridPrimitive>  // Hashing by both name and GridPrimitive type
    ,
    SharedConstChannel  // Shared channels, type information only as meta property
    ,
    ChannelCompare>  // Lesser operator on string-Primitve pairs
    BaseChannelMap;

class IVW_MODULE_DISCRETEDATA_API DataChannelMap {
public:
    DataChannelMap() = default;
    virtual ~DataChannelMap() = default;

    /** Default copy shares Channels */
    DataChannelMap(const DataChannelMap& copy) = default;

    // Methods
public:
    /** Add a new channel to the set
    *   @param channel Pointer to data, takes memory ownership
    *   @return Shared pointer for further handling
    */
    SharedChannel addChannel(Channel* channel);

    /** Add a new channel to the set
    *   @param channel Shared pointer to data, remains valid
    */
    void addChannel(SharedConstChannel channel);

    /** Returns the first channel from an unordered list.
    */
    SharedConstChannel getFirstChannel() const;

    /** Returns the first channel from an unordered list.
    */
    template <typename T>
    std::shared_ptr<const DataChannel<T>> getFirstChannel() const;

    /** Returns the specified channel, returns first instance found
    *   @param name Unique name of requested channel
    *   @param definedOn GridPrimitive type the channel is defined on, default 0D vertices
    */
    SharedConstChannel getChannel(const std::string& name,
                                  GridPrimitive definedOn = GridPrimitive::Vertex) const;

    /** Returns the specified channel if it is in the desired format, returns first instance found
    *   @param name Unique name of requested channel
    *   @param definedOn GridPrimitive type the channel is defined on, default 0D vertices
    */
    template <typename T>
    std::shared_ptr<const DataChannel<T>> getChannel(
        const std::string& name, GridPrimitive definedOn = GridPrimitive::Vertex) const;

    /** Returns the specified channel if it is in the desired format, returns first instance found
    *   @param name Unique name of requested channel
    *   @param definedOn GridPrimitive type the channel is defined on, default 0D vertices
    */
    template <typename T>
    std::shared_ptr<const DataChannel<T>> getChannel(
        const char* name, GridPrimitive definedOn = GridPrimitive::Vertex) const;

    /** Returns the specified buffer, converts to buffer or copies
    *   @param name Unique name of requested buffer
    *   @param definedOn GridPrimitive type the channel is defined on, default 0D vertices
    */
    template <typename T>
    std::shared_ptr<const BufferChannel<T>> getAsBuffer(
        const std::string& name, GridPrimitive definedOn = GridPrimitive::Vertex) const;

    /** Remove channel from set by shared pointer, data remains valid if shared outside
    *   Swaps position in vector
    *   @param channel Shared pointer to data
    *   @return Successfull - channel was saved in the set indeed
    */
    bool removeChannel(SharedConstChannel channel);

    /** Number of channels currently held
    */
    ind numChannels() const { return ChannelSet.size(); }

    // Attributes
public:
    /** Set of data channels
    *   Indexed by name and defining dimension (0D vertices, 1D edges etc).
    */
    BaseChannelMap ChannelSet;
};

/** \class DataSet
    \brief Data package containing structure by cell connectivity and data

    Conglomerate of data grid and several data channels assigned to grid dimensions.


    @author Anke Friederici and Tino Weinkauf
*/
class IVW_MODULE_DISCRETEDATA_API DataSet {

    // Construction / Deconstruction
public:
    DataSet() = default;
    virtual ~DataSet() = default;

    /** Default copy shares Channels and Connectivity */
    DataSet(const DataSet& copy) = default;

    // Methods
public:
    /// Returns a const typed shared pointer to the grid, if casting is possible.
    template <typename T>
    std::shared_ptr<const T> getGrid() const {
        return std::dynamic_pointer_cast<const T, const Connectivity>(Grid);
    }

    /// Returns a typed shared pointer to the grid, if casting is possible.
    template <typename T>
    std::shared_ptr<T> getGrid() {
        return std::dynamic_pointer_cast<T, Connectivity>(Grid);
    }

    // Attributes
public:
    /** Set of data channels
    *   Indexed by name and defining dimension (0D vertices, 1D edges etc).
    */
    DataChannelMap Channels;

    /** Connectivity of grid
    *   Several grid types are possible (rectlinear, structured, unstructured)
    */
    std::shared_ptr<Connectivity> Grid;
};

}  // namespace dd

template <>
struct DataTraits<dd::DataSet> {
    static std::string classIdentifier() { return "org.inviwo.DiscreteData"; }
    static std::string dataName() { return "DataSet"; }
    static uvec3 colorCode() { return uvec3(255, 144, 1); }
    static Document info(const dd::DataSet& data) {
        std::ostringstream oss;
        oss << "Data set with " << data.Channels.numChannels() << " channels.";
        if (!data.Channels.ChannelSet.empty())
            for (auto& channel : data.Channels.ChannelSet) {
                oss << "      " << channel.first.first << '[' << channel.second->getNumComponents()
                    << "][" << channel.second->getNumElements() << ']' << "(Dim "
                    << channel.first.second << ')';
            }
        Document doc;
        doc.append("p", oss.str());
        return doc;
    }
};

}  // namespace

#include "dataset.inl"
