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

#include <modules/discretedata/discretedatamoduledefine.h>

#include <modules/discretedata/channels/datachannel.h>
#include <modules/discretedata/channels/channelgetter.h>
#include <modules/discretedata/channels/cachedgetter.h>
// #include <inviwo/core/util/assertion.h>

namespace inviwo {
namespace discretedata {

/**
 * \brief Data channel extending values of two separate channels
 * Example:
 *   BaseChannel = {(1,2), (3,4)}, ExtendChannel = {5,6,7}
 *   ExtendedChannel = {(1,2,5), (3,4,5), (1,2,6), (3,4,6), (1,2,7), (3,4,7)}
 *
 * Holds shared_ptr to two DataChannels of the same base type.
 */
template <typename T, ind N>
class ExtendedChannel : public DataChannel<T, N> {

public:
    /**
     * \brief Direct construction
     * @param baseChannel The channel to extend
     * @param extendChannel The channel to extend by
     * @param name Name for the extended channel
     */
    ExtendedChannel(const std::shared_ptr<const BaseTypedChannel<T>>& baseChannel,
                    const std::shared_ptr<const BaseTypedChannel<T>>& extendChannel,
                    const std::string& name)
        : DataChannel<T, N>(
              name, baseChannel ? baseChannel->getGridPrimitiveType() : GridPrimitive::Vertex)
        , baseChannel_(baseChannel)
        , extendChannel_(extendChannel) {
        if (!baseChannel_ || !extendChannel_) {
            // ivwAssert(false, "Not two channels of correct type given.");
            return;
        }

        // ivwAssert(baseChannel_->getDataFormatId() == extendChannel_->getDataFormatId(),
        //           fmt::format("Scalar type \'{}\' of channels {} does not match \'{}\' of {}.",
        //                       DataFormatBase::get(baseChannel_->getDataFormatId())->getString(),
        //                       baseChannel_->getName(),
        //                       DataFormatBase::get(extendChannel_->getDataFormatId())->getString(),
        //                       extendChannel_->getName()));

        ind numTotalComponents =
            baseChannel_->getNumComponents() + extendChannel_->getNumComponents();

        // Check the number of components.
        // ivwAssert(numTotalComponents == N,
        //           fmt::format("Total number of components in channels ({}) does not match the
        //           size "
        //                       "provided to template ({}).",
        //                       numTotalComponents, N));
    }

    virtual ~ExtendedChannel() = default;

    virtual Channel* clone() const override { return new ExtendedChannel<T, N>(*this); }

    ind size() const override { return baseChannel_->size() * extendChannel_->size(); }

protected:
    /**
     * \brief Indexed point access, constant
     * Will write to the memory of dest via reinterpret_cast.
     * @param dest Position to write to, expect write of NumComponents many T
     * @param index Linear point index
     */
    void fillRaw(T* dest, const ind index, const ind numElements = 1) const override {
        if (index < 0 || index + numElements > baseChannel_->size() * extendChannel_->size()) {
            return;
        }
        const ind baseSize = baseChannel_->size();

        const ind startIndexInBase = index % baseSize;
        const ind numBaseElements = std::min(numElements, baseSize);

        const bool wrapsAround = (startIndexInBase + numElements > baseSize);
        const bool fullBaseChannel = numElements >= baseSize;

        ind indexInExtend = index / baseSize;
        const ind startIndexInBaseElements = fullBaseChannel ? startIndexInBase
                                             : wrapsAround   ? (index + numElements) % baseSize
                                                             : 0;

        // clang-format off
        /**
         * Memory to hold all elements we will need of the base channel.
         * We start with the first elements needed, possibly wrapping back to the first base channel element.
         * At maximum, all base channel elements are contained.
         * Examples for base channel containing A to F
         *   [A B C D E F]  all elements in order (iff num element is at least the base channel size)
         *   [D E]          subset in middle of base channel
         *   [A B C F]      elements from beginning and end of base channel (iff [F A B C] are needed in this order)
         */
        // clang-format on
        T* const baseChannelElements = new T[numBaseElements * baseChannel_->getNumComponents()];
        T* const currentExtendElement = new T[extendChannel_->getNumComponents()];

        const ind numElementsHead = wrapsAround ? baseSize - startIndexInBase : numElements;

        if (fullBaseChannel) {
            baseChannel_->fillRaw(baseChannelElements, 0, baseSize);
        } else {
            baseChannel_->fillRaw(baseChannelElements + startIndexInBaseElements, startIndexInBase,
                                  numElementsHead);
            if (wrapsAround) {
                baseChannel_->fillRaw(baseChannelElements, 0, startIndexInBaseElements);
            }
        }

        extendChannel_->fillRaw(currentExtendElement, indexInExtend, 1);

        // // DEBUG!
        // std::cout << "Base elements:" << std::endl;
        // for (size_t b = 0; b < numBaseElements * baseChannel_->getNumComponents(); ++b) {
        //     std::cout << " " << baseChannelElements[b] << std::endl;
        // }

        // std::cout << "Extend elements:" << std::endl;
        // for (size_t e = 0; e < extendChannel_->getNumComponents(); ++e) {
        //     std::cout << " " << currentExtendElement[e] << std::endl;
        // }
        // // !DEBUG

        T* baseChannelHead =
            baseChannelElements + startIndexInBaseElements * baseChannel_->getNumComponents();

        for (ind e = 0; e < numElementsHead; ++e) {
            memcpy(dest, baseChannelHead, sizeof(T) * baseChannel_->getNumComponents());
            baseChannelHead += baseChannel_->getNumComponents();
            dest += baseChannel_->getNumComponents();

            memcpy(dest, currentExtendElement, sizeof(T) * extendChannel_->getNumComponents());
            dest += extendChannel_->getNumComponents();
        }

        if (!wrapsAround) {
            delete[] baseChannelElements;
            delete[] currentExtendElement;
            return;
        }

        ind numRemainingElements = numElements - numElementsHead;

        while (numRemainingElements >= baseSize) {
            indexInExtend++;
            extendChannel_->fillRaw(currentExtendElement, indexInExtend, 1);

            baseChannelHead = baseChannelElements;

            for (ind e = 0; e < baseSize; ++e) {
                memcpy(dest, baseChannelHead, sizeof(T) * baseChannel_->getNumComponents());
                baseChannelHead += baseChannel_->getNumComponents();

                dest += baseChannel_->getNumComponents();
                memcpy(dest, currentExtendElement, sizeof(T) * extendChannel_->getNumComponents());
                dest += extendChannel_->getNumComponents();
            }

            numRemainingElements -= baseSize;
        }

        if (numRemainingElements > 0) {
            indexInExtend++;
            extendChannel_->fillRaw(currentExtendElement, indexInExtend, 1);

            // Wrapping around the end of the loaded data, going back to beginning.
            // if (baseChannelHead == baseChannelEnd)
            baseChannelHead = baseChannelElements;

            for (; numRemainingElements >= 0; numRemainingElements--) {
                memcpy(dest, baseChannelHead, sizeof(T) * baseChannel_->getNumComponents());
                baseChannelHead += baseChannel_->getNumComponents();

                dest += baseChannel_->getNumComponents();
                memcpy(dest, currentExtendElement, sizeof(T) * extendChannel_->getNumComponents());
                dest += extendChannel_->getNumComponents();
            }
        }
        delete[] baseChannelElements;
        delete[] currentExtendElement;
    }

protected:
    virtual CachedGetter<ExtendedChannel<T, N>>* newIterator() override {
        return new CachedGetter<ExtendedChannel<T, N>>(this);
    }

public:
    const std::shared_ptr<const BaseTypedChannel<T>> baseChannel_, extendChannel_;
};

namespace detail {
struct CreateExtendedChannelHelper {
    template <typename Result, typename T, ind N>
    Result operator()(const std::shared_ptr<const Channel>& baseChannel,
                      const std::shared_ptr<const Channel>& extendChannel,
                      const std::string& name) {
        return std::make_shared<ExtendedChannel<typename T::type, N>>(
            std::dynamic_pointer_cast<const BaseTypedChannel<typename T::type>>(baseChannel),
            std::dynamic_pointer_cast<const BaseTypedChannel<typename T::type>>(extendChannel),
            name);
    }
};

}  // namespace detail

std::shared_ptr<Channel> IVW_MODULE_DISCRETEDATA_API
createExtendedChannel(const std::shared_ptr<const Channel>& baseChannel,
                      const std::shared_ptr<const Channel>& extendChannel, const std::string& name);

}  // namespace discretedata
}  // namespace inviwo
