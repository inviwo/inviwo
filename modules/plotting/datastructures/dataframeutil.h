/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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

#ifndef IVW_DATAFRAMEUTIL_H
#define IVW_DATAFRAMEUTIL_H

#include <modules/plotting/plottingmoduledefine.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/io/serialization/serializable.h>
#include <modules/plotting/datastructures/dataframe.h>

namespace inviwo {

namespace dataframeutil {

struct IVW_MODULE_PLOTTING_API BufferCopyDispatcher {
    using type = std::shared_ptr<BufferBase>;
    template <class T>
    std::shared_ptr<BufferBase> dispatch(ivec2 range, std::shared_ptr<const BufferBase> from,
                                         std::shared_ptr<BufferBase> to, size_t dstStart) {
        typedef typename T::type F;

        BufferTarget target = from->getBufferTarget();

        if (target == BufferTarget::Index) {
            auto fram = from->getRepresentation<BufferRAM>();
            auto framp = dynamic_cast<const BufferRAMPrecision<F, BufferTarget::Index>*>(fram);
            if (framp) {
                auto tRam = to->getEditableRepresentation<BufferRAM>();
                auto tramp = dynamic_cast<BufferRAMPrecision<F, BufferTarget::Index>*>(tRam);
                auto& fromVec = framp->getDataContainer();
                auto& toVec = tramp->getDataContainer();

                if (range.x != range.y)
                    std::copy(fromVec.begin() + range.x, fromVec.begin() + range.y,
                              toVec.begin() + dstStart);
                return to;
            } else {
                throw inviwo::Exception(
                    "Data range not copied to index buffer",
                    IvwContextCustom("dataframeutil::BufferCopyDispatcher::dispatch"));
            }

        } else if (target == BufferTarget::Data) {
            auto fram = from->getRepresentation<BufferRAM>();
            auto framp = dynamic_cast<const BufferRAMPrecision<F, BufferTarget::Data>*>(fram);
            if (framp) {
                auto tRam = to->getEditableRepresentation<BufferRAM>();
                auto tramp = dynamic_cast<BufferRAMPrecision<F, BufferTarget::Data>*>(tRam);
                auto& fromVec = framp->getDataContainer();
                auto& toVec = tramp->getDataContainer();

                if (range.x != range.y)
                    std::copy(fromVec.begin() + range.x, fromVec.begin() + range.y,
                              toVec.begin() + dstStart);
                return to;
            } else {
                throw inviwo::Exception(
                    "Data range not copied to data buffer",
                    IvwContextCustom("dataframeutil::BufferCopyDispatcher::dispatch"));
            }
        }

        return nullptr;
    }
};

struct IVW_MODULE_PLOTTING_API BufferToStringDispatcher {
    using type = void;
    template <class T>
    void dispatch(std::shared_ptr<const BufferBase> from, std::vector<std::string>& strBuffer,
                  std::vector<std::string>& strDBuffer, std::string delimiter) {
        typedef typename T::type F;

        BufferTarget target = from->getBufferTarget();

        if (target == BufferTarget::Index || target == BufferTarget::Data) {
            auto fram = from->getRepresentation<BufferRAM>();
            auto frampI = dynamic_cast<const BufferRAMPrecision<F, BufferTarget::Index>*>(fram);
            auto frampD = dynamic_cast<const BufferRAMPrecision<F, BufferTarget::Data>*>(fram);

            if (frampI || frampD) {
                auto components = from->getDataFormat()->getComponents();
                auto& fromVec = (frampI) ? frampI->getDataContainer() : frampD->getDataContainer();

                // per component as double
                for (size_t i = 0; i < from->getSize(); i++) {
                    auto dstr = fram->getAsDVec4(i);
                    auto s = toString(dstr);

                    switch (components) {
                        case 1:
                            s = toString(dstr[0]);
                            break;
                        case 2:
                            s = toString(dstr[0]) + delimiter + toString(dstr[1]);
                            break;
                        case 3:
                            s = toString(dstr[0]) + delimiter + toString(dstr[1]) + delimiter +
                                toString(dstr[2]);
                            break;
                        case 4:
                            s = toString(dstr[0]) + delimiter + toString(dstr[1]) + delimiter +
                                toString(dstr[2]) + delimiter + toString(dstr[0]);
                            break;
                        default:
                            s = "";
                    }
                    strDBuffer.push_back(s);
                }

                // original precision and format (glm string)
                for (auto& b : fromVec) {
                    auto s = toString(b);
                    strBuffer.push_back(s);
                }
            } else {
                throw inviwo::Exception(
                    "IO failed for index buffer",
                    IvwContextCustom("dataframeutil::BufferCopyDispatcher::dispatch"));
            }
        }
    }
};

struct IVW_MODULE_PLOTTING_API BufferSerializerDispatcher {
    using type = void;
    template <class T>
    void dispatch(std::shared_ptr<const BufferBase> from, Serializer& serializer, std::string key,
                  std::string itemKey) {
        typedef typename T::type F;

        BufferTarget target = from->getBufferTarget();

        if (target == BufferTarget::Index || target == BufferTarget::Data) {
            auto fram = from->getRepresentation<BufferRAM>();
            auto frampI = dynamic_cast<const BufferRAMPrecision<F, BufferTarget::Index>*>(fram);
            auto frampD = dynamic_cast<const BufferRAMPrecision<F, BufferTarget::Data>*>(fram);

            if (frampI || frampD) {
                auto& fromVec = (frampI) ? frampI->getDataContainer() : frampD->getDataContainer();
                serializer.serialize(key, fromVec, itemKey);
            } else {
                throw inviwo::Exception(
                    "IO failed for index buffer",
                    IvwContextCustom("dataframeutil::BufferCopyDispatcher::dispatch"));
            }
        }
    }
};

std::shared_ptr<BufferBase> IVW_MODULE_PLOTTING_API
cloneBufferRange(std::shared_ptr<const BufferBase> buffer, ivec2 range);

void IVW_MODULE_PLOTTING_API copyBufferRange(std::shared_ptr<const BufferBase> src,
                                             std::shared_ptr<BufferBase> dst, ivec2 range,
                                             size_t dstStart = 0);

std::shared_ptr<plot::DataFrame> IVW_MODULE_PLOTTING_API
combineDataFrames(std::vector<std::shared_ptr<plot::DataFrame>> histogrameTimeDataFrame,
                  bool skipIndexColumn = false, std::string skipcol = "index");

}  // namespace dataframeutil

}  // namespace inviwo

#endif  // IVW_DATAFRAMEUTIL_H
