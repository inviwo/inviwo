/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include <inviwo/dataframe/datastructures/dataframeutil.h>
#include <inviwo/core/util/document.h>

#include <fmt/format.h>

namespace inviwo {

namespace dataframeutil {

std::shared_ptr<BufferBase> cloneBufferRange(std::shared_ptr<const BufferBase> buffer,
                                             ivec2 range) {
    if (range.y - range.x <= 0) {
        return nullptr;
    }

    return buffer->getRepresentation<BufferRAM>()->dispatch<std::shared_ptr<BufferBase>>(
        [&](auto typed) {
            using ValueType = util::PrecisionValueType<decltype(typed)>;
            auto newBuffer = std::make_shared<Buffer<ValueType>>();
            auto& vecOut = newBuffer->getEditableRAMRepresentation()->getDataContainer();
            auto& vecIn = typed->getDataContainer();

            vecOut.insert(vecOut.begin(), vecIn.begin() + range.x, vecIn.begin() + range.y + 1);
            return newBuffer;
        });
}

void copyBufferRange(std::shared_ptr<const BufferBase> src, std::shared_ptr<BufferBase> dst,
                     ivec2 range, size_t dstStart) {
    if (range.y - range.x <= 0) {
        return;
    }

    if (src->getDataFormat()->getId() == dst->getDataFormat()->getId()) {
        dst->getEditableRepresentation<BufferRAM>()->dispatch<void>([&](auto typed) {
            using ValueType = util::PrecisionValueType<decltype(typed)>;
            auto typedInBuf = static_cast<const Buffer<ValueType>*>(src.get());
            auto& vecIn = typedInBuf->getRAMRepresentation()->getDataContainer();
            auto& vecOut = typed->getDataContainer();

            vecOut.insert(vecOut.begin() + dstStart, vecIn.begin() + range.x,
                          vecIn.begin() + range.y + 1);
        });
    } else {
        throw inviwo::Exception("Data Formats needs to be of same type",
                                IVW_CONTEXT_CUSTOM("dataframeutil::copyBufferRange"));
    }
}

std::shared_ptr<DataFrame> combineDataFrames(std::vector<std::shared_ptr<DataFrame>> dataFrames,
                                             bool skipIndexColumn, std::string skipcol) {
    if (dataFrames.empty()) {
        throw inviwo::Exception("data frames vector is empty",
                                IVW_CONTEXT_CUSTOM("dataframeutil::combineDataFrames"));
    }
    if (dataFrames.size() == 1) {  // just one df, clone it;
        return std::make_shared<DataFrame>(*dataFrames.front().get());
    }

    size_t newSize = 0;
    for (auto& hf : dataFrames) {
        newSize += hf->getNumberOfRows();
    }

    if (newSize == 0) {
        throw inviwo::Exception("All Input data frames are empty",
                                IVW_CONTEXT_CUSTOM("dataframeutil::combineDataFrames"));
    }

    auto first = *dataFrames.front();
    {
        std::map<std::string, const DataFormatBase*> columnType;

        for (auto col : first) {
            if (skipIndexColumn && toLower(col->getHeader()) == skipcol) continue;
            columnType[col->getHeader()] = col->getBuffer()->getDataFormat();
        }

        for (auto df = dataFrames.begin() + 1; df != dataFrames.end(); ++df) {
            for (auto col : *df->get()) {
                if (skipIndexColumn && toLower(col->getHeader()) == skipcol) continue;
                auto it = columnType.find(col->getHeader());
                if (it == columnType.end()) {
                    std::ostringstream oss;
                    oss << "Column " << col->getHeader()
                        << " did not exist in first data frame but in at least one of the other "
                           "data frames";
                    throw inviwo::Exception(oss.str(),
                                            IVW_CONTEXT_CUSTOM("dataframeutil::combineDataFrames"));
                }
                if (it->second != col->getBuffer()->getDataFormat()) {
                    if (it == columnType.end()) {
                        std::ostringstream oss;
                        oss << "Column " << col->getHeader()
                            << " has different format in different data frames ("
                            << it->second->getString() << " and "
                            << col->getBuffer()->getDataFormat()->getSize() << ")";
                        throw inviwo::Exception(
                            oss.str(), IVW_CONTEXT_CUSTOM("dataframeutil::combineDataFrames"));
                    }
                }
            }
        }
    }

    std::unordered_map<std::string, std::shared_ptr<Column>> columns;
    std::shared_ptr<DataFrame> newDataFrame =
        std::make_shared<DataFrame>(static_cast<glm::u32>(newSize));
    for (auto col : first) {
        col->getBuffer()
            ->getRepresentation<BufferRAM>()
            ->dispatch<void, dispatching::filter::Scalars>([&](auto typedBuf) {
                IVW_UNUSED_PARAM(typedBuf);
                using ValueType = util::PrecisionValueType<decltype(typedBuf)>;
                columns[col->getHeader()] = newDataFrame->addColumn<ValueType>(col->getHeader());
            });
    }
    for (auto& data : dataFrames) {
        for (auto col : *(data.get())) {
            if (skipIndexColumn && toLower(col->getHeader()) == skipcol) continue;

            columns[col->getHeader()]
                ->getBuffer()
                ->getEditableRepresentation<BufferRAM>()
                ->dispatch<void>([&](auto typedBuf) {
                    using ValueType = util::PrecisionValueType<decltype(typedBuf)>;

                    auto typedBuffer =
                        static_cast<const Buffer<ValueType>*>(col->getBuffer().get());
                    auto& vec = typedBuffer->getRAMRepresentation()->getDataContainer();
                    typedBuf->getDataContainer().insert(typedBuf->getDataContainer().end(),
                                                        vec.begin(), vec.end());
                });
        }
    }
    return newDataFrame;
}

std::string createToolTipForRow(const DataFrame& dataframe, size_t rowId) {
    using H = utildoc::TableBuilder::Header;
    using P = Document::PathComponent;
    Document doc;
    doc.append("b", fmt::format("Data Point {}", rowId), {{"style", "color:white;"}});
    utildoc::TableBuilder tb(doc.handle(), P::end());
    for (size_t i = 0; i < dataframe.getNumberOfColumns(); i++) {
        tb(H(dataframe.getHeader(i)), dataframe.getColumn(i)->getAsString(rowId));
    }

    return doc;
}

}  // namespace dataframeutil

}  // namespace inviwo
