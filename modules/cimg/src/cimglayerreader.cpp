/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#include <modules/cimg/cimglayerreader.h>
#include <modules/cimg/cimgutils.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>

namespace inviwo {

CImgLayerReader::CImgLayerReader() : DataReaderType<Layer>() {
#ifdef cimg_use_jpeg
    addExtension(FileExtension("jpg", "Joint Photographic Experts Group"));
    addExtension(FileExtension("jpeg", "Joint Photographic Experts Group"));
#endif
    addExtension(FileExtension("bmp", "Windows bitmap"));
#ifdef cimg_use_openexr
    addExtension(FileExtension("exr", "OpenEXR"));
#endif
    addExtension(FileExtension("raw", "RAW"));
}

CImgLayerReader* CImgLayerReader::clone() const { return new CImgLayerReader(*this); }

namespace {

struct Dispatch {
    template <typename Result, typename T>
    std::shared_ptr<LayerRAM> operator()(void* data, const uvec2& dims) const {
        using F = typename T::type;

        auto swizzleMask = [](std::size_t numComponents) {
            switch (numComponents) {
                case 1:
                    return swizzlemasks::luminance;
                case 2:
                    return swizzlemasks::luminanceAlpha;
                case 3:
                    return swizzlemasks::rgb;
                case 4:
                default:
                    return swizzlemasks::rgba;
            }
        };

        return std::make_shared<LayerRAMPrecision<F>>(static_cast<F*>(data), dims, LayerType::Color,
                                                      swizzleMask(T::comp));
    }
};

}  // namespace

std::shared_ptr<Layer> CImgLayerReader::readData(const std::string& fileName) {
    if (!filesystem::fileExists(fileName)) {
        throw DataReaderException("Error could not find input file: " + fileName, IVW_CONTEXT);
    }

    uvec2 dimensions{0u};
    DataFormatId formatId = DataFormatId::NotSpecialized;
    void* data = cimgutil::loadLayerData(nullptr, fileName, dimensions, formatId, false);

    auto layerRAM =
        dispatching::dispatch<std::shared_ptr<LayerRepresentation>, dispatching::filter::All>(
            formatId, Dispatch{}, data, dimensions);

    return std::make_shared<Layer>(layerRAM);
}

}  // namespace inviwo
