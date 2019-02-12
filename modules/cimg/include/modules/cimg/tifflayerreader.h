/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/cimg/cimgmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/datastructures/diskrepresentation.h>
#include <inviwo/core/datastructures/image/layerdisk.h>

namespace inviwo {

class IVW_MODULE_CIMG_API TIFFLayerReaderException : public DataReaderException {
public:
    TIFFLayerReaderException(const std::string& message = "",
                             ExceptionContext context = ExceptionContext());
    virtual ~TIFFLayerReaderException() noexcept = default;
};

class IVW_MODULE_CIMG_API TIFFLayerReader : public DataReaderType<Layer> {
public:
    TIFFLayerReader();
    TIFFLayerReader(const TIFFLayerReader& rhs) = default;
    TIFFLayerReader& operator=(const TIFFLayerReader& that) = default;
    virtual TIFFLayerReader* clone() const override;
    virtual ~TIFFLayerReader() = default;

    virtual std::shared_ptr<Layer> readData(const std::string& fileName) override;

    template <typename Result, typename T>
    std::shared_ptr<Layer> operator()(void* data, size2_t dims, SwizzleMask swizzleMask) const {
        using F = typename T::type;
        auto layerRAM = std::make_shared<LayerRAMPrecision<F>>(
            static_cast<F*>(data), dims, LayerType::Color,
            swizzleMask);
        return std::make_shared<Layer>(layerRAM);
    }
};

}  // namespace inviwo
