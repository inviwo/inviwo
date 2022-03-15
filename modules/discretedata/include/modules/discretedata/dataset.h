/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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
#include <inviwo/core/datastructures/datatraits.h>

#include <modules/discretedata/channels/bufferchannel.h>
#include <modules/discretedata/connectivity/connectivity.h>
#include <modules/discretedata/connectivity/structuredgrid.h>
#include <modules/discretedata/sampling/datasetsampler.h>

namespace inviwo {
namespace discretedata {

struct DataSetInitializer {
    std::string name_;
    std::shared_ptr<const Connectivity> grid_;
    std::vector<std::shared_ptr<Channel>> channels_;
    std::vector<std::shared_ptr<const DataSetSamplerBase>> samplers_;
};

// Map used for storing and querying channels by name and GridPrimitive type.
// Ordering by both name and GridPrimitive type,
using DataChannelMap =
    std::map<std::pair<std::string, GridPrimitive>,
             std::shared_ptr<const Channel>,                     // Shared base channels pointers
             dd_util::PairCompare<std::string, GridPrimitive>>;  // Lesser operator on
                                                                 // String-Primitive pairs

using SamplerMap = std::map<std::string, std::shared_ptr<const DataSetSamplerBase>>;

/**
 * \brief Data package containing structure by cell connectivity and data
 * Conglomerate of data grid and several data channels assigned to grid dimensions.
 */
class IVW_MODULE_DISCRETEDATA_API DataSet {
public:
    /**
     * \brief Create a DataSet from an existing grid
     * @param name The (preferably unique) name of this DataSet
     * @param grid Existing grid to base the DataSet on
     */
    DataSet(const std::string& name, std::shared_ptr<const Connectivity> grid)
        : grid_(grid), name_(name) {}

    /**
     * \brief Create a DataSet on an nD StructuredGrid
     * @param name The (preferably unique) name of this DataSet
     * @param numVertices Number of vertices in N dimensions
     */
    template <size_t N>
    DataSet(const std::string& name, const std::array<ind, N>& numVertices)
        : grid_(std::make_shared<StructuredGrid<static_cast<ind>(N)>>(numVertices)), name_(name) {}

    /**
     * \brief Create a DataSet on an nD StructuredGrid
     * @param name The (preferably unique) name of this DataSet
     * @param val0 Required size of first dimension
     * @param valX Further N-1 sizes
     */
    template <typename... IND>
    DataSet(const std::string& name, ind val0, IND... valX)
        : grid_(std::make_shared<StructuredGrid<sizeof...(IND) + 1>>(val0, valX...)), name_(name) {}

    /**
     * \brief Create a DataSet from an existing grid and channel list
     * @param data Existing grid and channels to base the DataSet on
     */
    DataSet(const DataSetInitializer& data) : grid_(data.grid_), name_(data.name_) {
        for (auto& channel : data.channels_)
            addChannel(std::const_pointer_cast<const Channel, Channel>(channel));
        for (auto& sampler : data.samplers_) addSampler(sampler);
    }

    virtual ~DataSet() = default;

    /**
     * Default copy shares Channels and Connectivity
     */
    DataSet(const DataSet& copy) = default;

public:
    /**
     * Returns a const typed shared pointer to the grid, if casting is possible.
     */
    template <typename G>
    const std::shared_ptr<const G> getGrid() const {
        return std::dynamic_pointer_cast<const G, const Connectivity>(grid_);
    }

    /**
     * Returns a shared pointer to the virtual grid.
     */
    std::shared_ptr<const Connectivity> getGrid() const { return grid_; }

    const std::string& getName() const { return name_; }

    // Channels

    bool hasChannel(std::shared_ptr<const Channel> channel) const;

    /**
     * Add a new channel to the set
     * @param channel Pointer to data, takes memory ownership
     * @return Shared pointer for further handling
     */
    std::shared_ptr<const Channel> addChannel(const Channel* channel);

    /**
     * Add a new channel to the set
     * @param channel Shared pointer to data, remains valid
     */
    void addChannel(std::shared_ptr<const Channel> channel);

    /**
     * Returns the channel list (map from name and primitive to channel shared pointer).
     */
    DataChannelMap getChannels() const;

    /**
     * Returns the first channel from an unordered list.
     */
    std::shared_ptr<const Channel> getFirstChannel() const;

    /**
     * Returns the first channel from an unordered list.
     */
    template <typename T, ind N>
    std::shared_ptr<const DataChannel<T, N>> getFirstChannel() const;

    /**
     * Returns the specified channel, returns first instance found
     * @param name Unique name of requested channel
     * @param definedOn GridPrimitive type the channel is defined on, default 0D vertices
     */
    std::shared_ptr<const Channel> getChannel(
        const std::string& name, GridPrimitive definedOn = GridPrimitive::Vertex) const;

    /**
     * Returns the specified channel if it is in the desired format, returns first instance found
     * @param key Unique name and GridPrimitive type the channel is defined on
     */
    std::shared_ptr<const Channel> getChannel(
        const std::pair<std::string, GridPrimitive>& key) const;

    /**
     * Returns the specified channel if it is in the desired format, returns first instance found
     * @param name Unique name of requested channel
     * @param definedOn GridPrimitive type the channel is defined on, default 0D vertices
     */
    template <typename T, ind N>
    std::shared_ptr<const DataChannel<T, N>> getChannel(
        const std::string& name, GridPrimitive definedOn = GridPrimitive::Vertex) const;

    /**
     * Returns the specified buffer, converts to buffer or copies
     * @param name Unique name of requested buffer
     * @param definedOn GridPrimitive type the channel is defined on, default 0D vertices
     */
    template <typename T, ind N>
    std::shared_ptr<const BufferChannel<T, N>> getAsBuffer(
        const std::string& name, GridPrimitive definedOn = GridPrimitive::Vertex) const;

    /**
     * Remove channel from set by shared pointer, data remains valid if shared outside
     * Swaps position in vector
     * @param channel Shared pointer to data
     * @return Successful - channel was saved in the set indeed
     */
    bool removeChannel(std::shared_ptr<const Channel> channel);

    /**
     * Number of channels currently held
     */
    ind size() const { return channels_.size(); }

    std::vector<std::pair<std::string, GridPrimitive>> getChannelNames() const;

    DataChannelMap::const_iterator cbegin() const { return channels_.cbegin(); }
    DataChannelMap::const_iterator cend() const { return channels_.cend(); }

    /**
     * Returns the list of DataSetSamplers (map from long name to Sampler shared pointer).
     */
    const SamplerMap& getSamplers() const;

    /**
     * Try to add a new sampler.
     * Will be rejected if the sampler is based on a channel not in this dataset
     * or not of maximal dimension (e.g., a 2D sampler for a 3D dataset).
     * @return Successful - a valid sampler for this dataset, now available in the sampler list.
     */
    bool addSampler(std::shared_ptr<const DataSetSamplerBase> sampler);

protected:
    /**
     * Set of data channels
     * Indexed by name and defining dimension (0D vertices, 1D edges etc).
     */
    DataChannelMap channels_;

    /**
     * Connectivity of grid
     * Several grid types are possible (rectlinear, structured, unstructured)
     */
    const std::shared_ptr<const Connectivity> grid_;

    /**
     * Samplers for various position channels.
     * Possibly including spatial datastructures for quick cell location.
     * Can be used to create inviwo SpatialSamplers when a channel to sample from is selected.
     */
    SamplerMap samplers_;

    /**
     * The identifying name of this DataSet.
     * Will be displayed to the user, and can be used to loosely identify this DataSet.
     */
    const std::string name_;
};

template <typename T, ind N>
std::shared_ptr<const DataChannel<T, N>> DataSet::getFirstChannel() const {
    std::shared_ptr<const Channel> channel = getFirstChannel();
    return std::dynamic_pointer_cast<const DataChannel<T, N>, const Channel>(channel);
}

template <typename T, ind N>
std::shared_ptr<const DataChannel<T, N>> DataSet::getChannel(const std::string& name,
                                                             GridPrimitive definedOn) const {
    std::shared_ptr<const Channel> channel = getChannel(name.c_str(), definedOn);
    return std::dynamic_pointer_cast<const DataChannel<T, N>, const Channel>(channel);
}

template <typename T, ind N>
std::shared_ptr<const BufferChannel<T, N>> DataSet::getAsBuffer(const std::string& name,
                                                                GridPrimitive definedOn) const {
    std::shared_ptr<const Channel> channel = getChannel(name, definedOn);

    // Data is not present in this data set.
    if (!channel) return std::shared_ptr<const BufferChannel<T, N>>(nullptr);

    // Try to cast the channel to DataChannel<T, N>.
    // Return empty shared_ptr if unsuccessful.
    std::shared_ptr<const DataChannel<T, N>> dataChannel =
        std::dynamic_pointer_cast<const DataChannel<T, N>, const Channel>(channel);

    // Check for nullptr inside
    if (!dataChannel) return std::shared_ptr<const BufferChannel<T, N>>(nullptr);

    // Try to cast the channel to buffer directly.
    // If successful, return a shared pointer to the buffer directly.
    // (Shared pointer remains valid and shares the refereNe counter).
    std::shared_ptr<const BufferChannel<T, N>> bufferChannel =
        std::dynamic_pointer_cast<const BufferChannel<T, N>, const Channel>(channel);

    // Check for nullptr inside.
    if (bufferChannel) return bufferChannel;

    return std::make_shared<const BufferChannel<T, N>>(*dataChannel);
}

}  // namespace discretedata

template <>
struct DataTraits<discretedata::DataSet> {
    static std::string classIdentifier() { return "org.inviwo.DiscreteData"; }
    static std::string dataName() { return "DataSet"; }
    static uvec3 colorCode() { return uvec3(255, 144, 1); }
    static Document info(const discretedata::DataSet& data) {
        Document doc;
        // doc.append("p", oss.str());
        // std::ostringstream oss;
        auto grid = data.getGrid();
        doc.append("p", fmt::format("{} - {} channels and {} samplers.", data.getName(),
                                    data.size(), data.getSamplers().size()));
        doc.append("p",
                   fmt::format("{} with {} vertices and {} {} cells.", grid->getIdentifier(),
                               grid->getNumElements(), grid->getNumElements(grid->getDimension()),
                               primitiveName(grid->getDimension())));

        // Channels.
        doc.append("p", "");
        doc.append("b", "Channels", {{"style", "color:white;"}});
        auto channelKeyList = data.getChannelNames();
        if (channelKeyList.size() != 0)
            for (auto& channelKey : channelKeyList) {
                auto channel = data.getChannel(channelKey);
                doc.append("p", fmt::format("  {}[{}] [{}] ({}D)", channelKey.first,
                                            channel->getNumComponents(), channel->size(),
                                            (int)channelKey.second));
            }

        // Samplers.
        doc.append("p", "");
        doc.append("b", "Samplers", {{"style", "color:white;"}});
        for (auto& sampler : data.getSamplers()) {
            doc.append("p", fmt::format("   {}", sampler.second->getIdentifier()));
        }
        return doc;
    }
};

}  // namespace inviwo
