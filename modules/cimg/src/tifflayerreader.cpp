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

#include <modules/cimg/tifflayerreader.h>

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/raiiutils.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

#include <modules/cimg/cimgutils.h>

#include <tiff/libtiff/tiffio.h>

#include <sstream>

namespace inviwo {

TIFFLayerReaderException::TIFFLayerReaderException(const std::string& message,
                                                   ExceptionContext context)
    : DataReaderException(message, context) {}

TIFFLayerReader::TIFFLayerReader() : DataReaderType<Layer>() {
#ifdef cimg_use_tiff
    addExtension(FileExtension("tif", "TIFF (Tagged Image File Format)"));
    addExtension(FileExtension("tiff", "TIFF (Tagged Image File Format)"));
#endif
}

TIFFLayerReader* TIFFLayerReader::clone() const { return new TIFFLayerReader(*this); }

std::shared_ptr<inviwo::Layer> TIFFLayerReader::readData(const std::string& fileName) {
    if (!filesystem::fileExists(fileName))
        throw TIFFLayerReaderException("Failed to open file for reading, " + fileName, IVW_CONTEXT);

    auto header = cimgutil::getTIFFHeader(fileName);
    auto data = cimgutil::loadTIFFLayerData(nullptr, fileName, header, false);

    auto layer = dispatching::dispatch<std::shared_ptr<Layer>, dispatching::filter::All>(
        header.format->getId(), *this, data, size2_t{header.dimensions}, header.swizzleMask);

    return layer;
}

}  // namespace inviwo
