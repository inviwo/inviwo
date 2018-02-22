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

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <modules/plotting/datastructures/dataframeutil.h>

namespace inviwo {

namespace dataframeutil {

std::shared_ptr<BufferBase> cloneBufferRange(std::shared_ptr<const BufferBase> buffer,
                                             ivec2 range) {
    if (range.y - range.x <= 0) {
        return nullptr;
    }

    return buffer->getRepresentation<BufferRAM>()->dispatch<std::shared_ptr<BufferBase>>(

        [&](auto typed) {
            using ValueType = util::PrecsionValueType<decltype(typed)>;
            auto newBuffer = std::make_shared<Buffer<ValueType>>();
            auto &vecOut = newBuffer->getEditableRAMRepresentation()->getDataContainer();
            auto &vecIn = typed->getDataContainer();

            vecOut.insert(vecOut.begin(), vecIn.begin() + range.x, vecIn.begin() + range.y + 1);
            return newBuffer;
        }

    );
}

void copyBufferRange(std::shared_ptr<const BufferBase> src, std::shared_ptr<BufferBase> dst,
                     ivec2 range, size_t dstStart) {
    if (range.y - range.x <= 0) {
        return;
    }

    if (src->getDataFormat()->getId() == dst->getDataFormat()->getId()) {
        dst->getEditableRepresentation<BufferRAM>()->dispatch<void>([&](auto typed) {
            using ValueType = util::PrecsionValueType<decltype(typed)>;
            auto typedInBuf = static_cast<const Buffer<ValueType> *>(src.get());
            auto &vecIn = typedInBuf->getRAMRepresentation()->getDataContainer();
            auto &vecOut = typed->getDataContainer();

            vecOut.insert(vecOut.begin() + dstStart, vecIn.begin() + range.x,
                          vecIn.begin() + range.y + 1);
        });
    }
}

std::shared_ptr<plot::DataFrame> combineDataFrames(
    std::vector<std::shared_ptr<plot::DataFrame>> dataFrames, bool skipIndexColumn,
    std::string skipcol) {
    if (dataFrames.size() == 0) return nullptr;
    if (dataFrames.size() == 1) {  // just one df, clone it;
        return std::make_shared<plot::DataFrame>(*dataFrames.front().get());
    }

    size_t newSize = 0;
    for (auto &hf : dataFrames) {
        newSize += hf->getNumberOfRows();
    }

    if (newSize == 0) return nullptr;

    auto first = *dataFrames.front();
    {
        std::map<std::string, const DataFormatBase *> columnType;

        for (auto col : first) {
            if (skipIndexColumn && toLower(col->getHeader()) == skipcol) continue;
            columnType[col->getHeader()] = col->getBuffer()->getDataFormat();
        }

        for (auto df = dataFrames.begin() + 1; df != dataFrames.end(); ++df) {
            for (auto col : *df->get()) {
                if (skipIndexColumn && toLower(col->getHeader()) == skipcol) continue;
                auto it = columnType.find(col->getHeader());
                if (it == columnType.end()) {
                    LogErrorCustom("combineDataFrames", "Column " << col->getHeader()
                                                                  << " did not exist in first "
                                                                     "dataframe but in at least "
                                                                     "one of the other dataframes");
                    return nullptr;
                }
                if (it->second != col->getBuffer()->getDataFormat()) {
                    if (it == columnType.end()) {
                        LogErrorCustom("combineDataFrames",
                                       "Column "
                                           << col->getHeader()
                                           << " as different format in differetn data frames ("
                                           << it->second->getString() << " and "
                                           << col->getBuffer()->getDataFormat()->getSize());
                        return nullptr;
                    }
                }
            }
        }
    }

    std::unordered_map<std::string, std::shared_ptr<plot::Column>> columns;
    std::shared_ptr<plot::DataFrame> newDataFrame =
        std::make_shared<plot::DataFrame>(static_cast<glm::u32>(newSize));
    for (auto col : first) {
        col->getBuffer()
            ->getRepresentation<BufferRAM>()
            ->dispatch<void, dispatching::filter::Scalars>([&](auto typedBuf) {
                using ValueType = util::PrecsionValueType<decltype(typedBuf)>;
                columns[col->getHeader()] = newDataFrame->addColumn<ValueType>(col->getHeader());
            });
    }
    for (auto &data : dataFrames) {
        for (auto col : *(data.get())) {
            if (skipIndexColumn && toLower(col->getHeader()) == skipcol) continue;

            columns[col->getHeader()]
                ->getBuffer()
                ->getEditableRepresentation<BufferRAM>()
                ->dispatch<void>([&](auto typedBuf) {
                    using ValueType = util::PrecsionValueType<decltype(typedBuf)>;

                    auto typedBuffer =
                        static_cast<const Buffer<ValueType> *>(col->getBuffer().get());
                    auto &vec = typedBuffer->getRAMRepresentation()->getDataContainer();
                    typedBuf->getDataContainer().insert(typedBuf->getDataContainer().end(),
                                                        vec.begin(), vec.end());
                });
        }
    }
    return newDataFrame;
}

}  // namespace dataframeutil

}  // namespace inviwo
