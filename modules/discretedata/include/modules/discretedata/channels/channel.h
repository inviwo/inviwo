/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2021 Inviwo Foundation
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
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/metadata/metadataowner.h>

#include <modules/discretedata/discretedatatypes.h>
#include <modules/discretedata/channels/channeldispatching.h>

namespace inviwo {
namespace discretedata {

template <typename T, ind N>
class DataChannel;

/**
 * \brief An untyped scalar or vector component of a data set.
 *
 * General version of a DataChannel for use in general containers
 * (see DataSet).
 */
class IVW_MODULE_DISCRETEDATA_API Channel : public MetaDataOwner {
    // friend class CombinedChannel;

public:
    /**
     * \brief Direct construction
     * @param numComponents Size of vector at each position
     * @param name Name associated with the channel
     * @param dataFormat Data format
     * @param definedOn GridPrimitive the data is defined on, default: 0D vertices
     */
    Channel(ind numComponents, const std::string& name, DataFormatId dataFormat,
            GridPrimitive definedOn = GridPrimitive::Vertex);

    virtual ~Channel() = default;

    virtual Channel* clone() const = 0;

    const std::string getName() const;

    void setName(const std::string&);

    GridPrimitive getGridPrimitiveType() const;

    /** Gets the scalar data format type, i.e. Float64 for a vec3 channel. **/
    DataFormatId getDataFormatId() const;

    ind getNumComponents() const;

    virtual ind size() const = 0;

    virtual double getInvalidValueDouble() const = 0;

    virtual void setInvalidValueDouble(double val) = 0;

protected:
    /**
     * Sets the "GridPrimitiveType" meta data
     * Should be constant, only DataSet is allowed to write.
     */
    void setGridPrimitiveType(GridPrimitive);

    void setDataFormatId(DataFormatId);

    /**
     * Sets the "NumComponents" meta data
     * Should be constant, only DataSet is allowed to write.
     */
    void setNumComponents(ind);

    virtual void fillRaw(void* dest, ind index, ind numElements = 1) const = 0;

public:
    /**
     * Dispatching a function that gets a templated DataChannel as first argument.
     */
    template <typename Result, template <class> class Predicate, ind Min, ind Max,
              typename Callable, typename... Args>
    auto dispatch(Callable&& callable, Args&&... args) -> Result;

    /**
     * Dispatching a function that gets a templated DataChannel as first argument.
     */
    template <typename Result, template <class> class Predicate, ind Min, ind Max,
              typename Callable, typename... Args>
    auto dispatch(Callable&& callable, Args&&... args) const -> Result;

    /**
     * Dispatching a function that gets a templated DataChannel as first argument.
     * Standard filter and dimension range.
     */
    template <typename Result, typename Callable, typename... Args>
    auto dispatch(Callable&& callable, Args&&... args) -> Result;

    /**
     * Dispatching a function that gets a templated DataChannel as first argument.
     * Standard filter and dimension range.
     */
    template <typename Result, typename Callable, typename... Args>
    auto dispatch(Callable&& callable, Args&&... args) const -> Result;

    /**
     * Dispatching a function that gets a templated DataChannel as first argument.
     * Standard filter and dimension range.
     */
    // template <typename Result, typename Callable, typename... Args>
    // static auto dispatchSharedPointer(std::shared_ptr<Channel> channel, Callable&& callable,
    //                                   Args&&... args) -> Result;
    /**
     * Dispatching a function that gets a templated DataChannel as first argument.
     * Standard filter and dimension range.
     */
    template <typename Result, typename Callable, typename... Args>
    static auto dispatchSharedPointer(std::shared_ptr<const Channel> channel, Callable&& callable,
                                      Args&&... args) -> Result;

    virtual std::shared_ptr<Channel> toBufferChannel() const = 0;

private:
    std::string name_;
    const DataFormatBase* format_;
    GridPrimitive grid_;
    ind numComponents_;
};

// template <typename Result, template <class> class Predicate, ind Min, ind Max, typename Callable,
//           typename... Args>
// auto channeldispatching::dispatch(DataFormatId format, ind numComponents, Callable &&obj, Args
// &&... args) -> Result;

namespace detail_dd {
struct ChannelDispatcher {
    template <typename Result, typename T, ind N, typename Callable, typename... Args>
    Result operator()(Callable&& obj, Channel* channel, Args... args) {
        return obj(dynamic_cast<DataChannel<typename T::type, N>*>(channel),
                   std::forward<Args>(args)...);
    }
};

struct ChannelConstDispatcher {
    template <typename Result, typename T, ind N, typename Callable, typename... Args>
    Result operator()(Callable&& obj, const Channel* channel, Args... args) {
        static_assert(N > 0);
        return obj(dynamic_cast<const DataChannel<typename T::type, N>*>(channel),
                   std::forward<Args>(args)...);
    }
};

struct ChannelSharedDispatcher {
    template <typename Result, typename T, ind N, typename Callable, typename... Args>
    Result operator()(Callable&& obj, std::shared_ptr<Channel> channel, Args... args) {
        return obj(std::dynamic_pointer_cast<DataChannel<typename T::type, N>>(channel),
                   std::forward<Args>(args)...);
    }
};

struct ChannelSharedConstDispatcher {
    template <typename Result, typename T, ind N, typename Callable, typename... Args>
    Result operator()(Callable&& obj, std::shared_ptr<const Channel> channel, Args... args) {
        static_assert(N > 0);
        return obj(std::dynamic_pointer_cast<const DataChannel<typename T::type, N>>(channel),
                   std::forward<Args>(args)...);
    }
};
}  // namespace detail_dd

// Variants with filter and dimension range.

template <typename Result, template <class> class Predicate, ind Min, ind Max, typename Callable,
          typename... Args>
auto Channel::dispatch(Callable&& callable, Args&&... args) -> Result {
    detail_dd::ChannelDispatcher dispatcher;
    return channeldispatching::dispatch<Result, Predicate, Min, Max>(
        getDataFormatId(), getNumComponents(), dispatcher, std::forward<Callable>(callable), this,
        std::forward<Args>(args)...);
}

template <typename Result, template <class> class Predicate, ind Min, ind Max, typename Callable,
          typename... Args>
auto Channel::dispatch(Callable&& callable, Args&&... args) const -> Result {
    detail_dd::ChannelConstDispatcher dispatcher;
    return channeldispatching::dispatch<Result, Predicate, Min, Max>(
        getDataFormatId(), getNumComponents(), dispatcher, std::forward<Callable>(callable), this,
        std::forward<Args>(args)...);
}

// Variants filtering for Scalars x [1,4].

template <typename Result, typename Callable, typename... Args>
auto Channel::dispatch(Callable&& callable, Args&&... args) -> Result {
    detail_dd::ChannelDispatcher dispatcher;
    return channeldispatching::dispatch<Result, dispatching::filter::Scalars, 1, 4>(
        getDataFormatId(), getNumComponents(), dispatcher, std::forward<Callable>(callable), this,
        std::forward<Args>(args)...);
}

template <typename Result, typename Callable, typename... Args>
auto Channel::dispatch(Callable&& callable, Args&&... args) const -> Result {
    detail_dd::ChannelConstDispatcher dispatcher;
    return channeldispatching::dispatch<Result, dispatching::filter::Scalars, 1, 4>(
        getDataFormatId(), getNumComponents(), dispatcher, std::forward<Callable>(callable), this,
        std::forward<Args>(args)...);
}

// template <typename Result, typename Callable, typename... Args>
// static auto dispatchSharedPointer(std::shared_ptr<Channel> channel, Callable&& callable,
//                                   Args&&... args) -> Result {
//     detail_dd::ChannelSharedDispatcher dispatcher;
//     return channeldispatching::dispatch<Result, dispatching::filter::Scalars, 1, 4>(
//         channel->getDataFormatId(), channel->getNumComponents(), dispatcher,
//         std::forward<Callable>(callable), channel, std::forward<Args>(args)...);
// }

template <typename Result, typename Callable, typename... Args>
auto Channel::dispatchSharedPointer(std::shared_ptr<const Channel> channel, Callable&& callable,
                                    Args&&... args) -> Result {
    detail_dd::ChannelSharedConstDispatcher dispatcher;
    return channeldispatching::dispatch<Result, dispatching::filter::Scalars, 1, 4>(
        channel->getDataFormatId(), channel->getNumComponents(), dispatcher,
        std::forward<Callable>(callable), channel, std::forward<Args>(args)...);
}

}  // namespace discretedata
}  // namespace inviwo
