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

#include <modules/cimg/tifflayerreader.h>

#include <inviwo/core/datastructures/image/layer.h>     // for Layer, DataReaderType
#include <inviwo/core/datastructures/image/layerram.h>  // IWYU pragma: keep
#include <inviwo/core/io/datareader.h>                  // for DataReaderType
#include <inviwo/core/util/fileextension.h>             // for FileExtension
#include <inviwo/core/util/formatdispatching.h>         // for dispatch, All
#include <inviwo/core/util/formats.h>                   // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                    // for size2_t
#include <modules/cimg/cimgutils.h>  // for TIFFHeader, getTIFFHeader, loadTIFFL...

namespace inviwo {

TIFFLayerReader::TIFFLayerReader() : DataReaderType<Layer>() {
#ifdef cimg_use_tiff
    addExtension(FileExtension("tif", "TIFF (Tagged Image File Format)"));
    addExtension(FileExtension("tiff", "TIFF (Tagged Image File Format)"));
#endif
}

TIFFLayerReader* TIFFLayerReader::clone() const { return new TIFFLayerReader(*this); }

std::shared_ptr<inviwo::Layer> TIFFLayerReader::readData(const std::filesystem::path& fileName) {
    checkExists(fileName);

    auto header = cimgutil::getTIFFHeader(fileName);
    auto data = cimgutil::loadTIFFLayerData(nullptr, fileName, header, false);

    auto layer = dispatching::singleDispatch<std::shared_ptr<Layer>, dispatching::filter::All>(
        header.format->getId(), [&]<typename T>() -> std::shared_ptr<Layer> {
            auto layerRAM = std::make_shared<LayerRAMPrecision<T>>(
                static_cast<T*>(data), size2_t{header.dimensions}, LayerType::Color,
                header.swizzleMask);
            return std::make_shared<Layer>(layerRAM);
        });

    return layer;
}

}  // namespace inviwo
