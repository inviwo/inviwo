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

#ifndef IVW_DATAFRAMEUTIL_H
#define IVW_DATAFRAMEUTIL_H

#include <inviwo/dataframe/dataframemoduledefine.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/dataframe/datastructures/dataframe.h>

namespace inviwo {

namespace dataframeutil {

std::shared_ptr<BufferBase> IVW_MODULE_DATAFRAME_API
cloneBufferRange(std::shared_ptr<const BufferBase> buffer, ivec2 range);

void IVW_MODULE_DATAFRAME_API copyBufferRange(std::shared_ptr<const BufferBase> src,
                                              std::shared_ptr<BufferBase> dst, ivec2 range,
                                              size_t dstStart = 0);

std::shared_ptr<DataFrame> IVW_MODULE_DATAFRAME_API
combineDataFrames(std::vector<std::shared_ptr<DataFrame>> histogramTimeDataFrame,
                  bool skipIndexColumn = false, std::string skipcol = "index");

std::string IVW_MODULE_DATAFRAME_API createToolTipForRow(const DataFrame &dataframe, size_t rowId);

}  // namespace dataframeutil

}  // namespace inviwo

#endif  // IVW_DATAFRAMEUTIL_H
