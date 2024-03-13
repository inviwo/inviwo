/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/imagetypes.h>  // for SwizzleMask
#include <inviwo/core/util/formats.h>                     // for DataFormatId, DataFormatBase (p...
#include <inviwo/core/util/glmvec.h>                      // for uvec2, size3_t, dvec2

#include <memory>       // for unique_ptr
#include <string>       // for string
#include <string_view>  // for string_view
#include <vector>       // for vector
#include <filesystem>

namespace inviwo {
class LayerRAM;
class VolumeRAM;

namespace cimgutil {

enum class InterpolationType : int {
    RawMemory = -1,       // raw memory resizing.
    NoInterpolation = 0,  // additional space is filled according to boundary_conditions.
    Nearest = 1,          // nearest - neighbor interpolation.
    Moving = 2,           // moving average interpolation.
    Linear = 3,           // linear interpolation.
    Grid = 4,             // grid interpolation.
    Cubic = 5,            // cubic interpolation.
    Lanczos = 6           // lanczos interpolation.
};

enum class ConsiderAspectRatio { No, Yes };

enum class TIFFResolutionUnit { None, Inch, Centimeter };

struct IVW_MODULE_CIMG_API TIFFHeader {
    const DataFormatBase* format;
    size3_t dimensions;
    dvec2 resolution;
    TIFFResolutionUnit resolutionUnit;
    SwizzleMask swizzleMask;
};

IVW_MODULE_CIMG_API TIFFHeader getTIFFHeader(const std::filesystem::path& filename);

/**
 * Loads layer from a specified filePath.
 **/
IVW_MODULE_CIMG_API std::shared_ptr<LayerRAM> loadLayer(const std::filesystem::path& filePath);
IVW_MODULE_CIMG_API std::shared_ptr<LayerRAM> loadLayerTiff(const std::filesystem::path& filePath);

/**
 * Loads volume from a specified filePath.
 **/
IVW_MODULE_CIMG_API std::shared_ptr<VolumeRAM> loadVolume(const std::filesystem::path& filePath);
IVW_MODULE_CIMG_API std::shared_ptr<VolumeRAM> loadVolume(const std::filesystem::path& filePath,
                                                          const DataFormatBase* format,
                                                          size3_t dims);
IVW_MODULE_CIMG_API void updateVolume(VolumeRAM& volume, const std::filesystem::path& filePath);

IVW_MODULE_CIMG_API void saveLayer(const LayerRAM& layer, const std::filesystem::path& filePath);
IVW_MODULE_CIMG_API void saveLayer(const LayerRAM& layer, std::vector<unsigned char>& dst,
                                   std::string_view extension);

IVW_MODULE_CIMG_API bool rescaleLayerRamToLayerRam(const LayerRAM* source, LayerRAM* target);

IVW_MODULE_CIMG_API bool rescaleLayerRamToLayerRam(
    const LayerRAM* source, LayerRAM* target, cimgutil::InterpolationType interpolation,
    cimgutil::ConsiderAspectRatio aspectRatio = cimgutil::ConsiderAspectRatio::Yes);

IVW_MODULE_CIMG_API std::string getLibJPGVersion();
IVW_MODULE_CIMG_API std::string getOpenEXRVersion();

}  // namespace cimgutil

}  // namespace inviwo
