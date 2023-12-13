/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <modules/cimg/cimgmoduledefine.h>  // for IVW_MODULE_CIMG_API

#include <inviwo/core/datastructures/image/imagetypes.h>  // for LayerType, LayerType::Color
#include <inviwo/core/datastructures/image/layer.h>       // for DataReaderType
#include <inviwo/core/io/datareader.h>                    // for DataReaderType
#include <inviwo/core/util/glmvec.h>                      // for size2_t

#include <memory>       // for shared_ptr, make_shared
#include <string_view>  // for string_view

namespace inviwo {
template <typename T>
class LayerRAMPrecision;

class IVW_MODULE_CIMG_API TIFFLayerReader : public DataReaderType<Layer> {
public:
    TIFFLayerReader();
    TIFFLayerReader(const TIFFLayerReader& rhs) = default;
    TIFFLayerReader& operator=(const TIFFLayerReader& that) = default;
    virtual TIFFLayerReader* clone() const override;
    virtual ~TIFFLayerReader() = default;

    virtual std::shared_ptr<Layer> readData(const std::filesystem::path& fileName) override;
};

}  // namespace inviwo
