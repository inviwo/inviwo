/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2022 Inviwo Foundation
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

#include <modules/cimg/cimgmoduledefine.h>                // for IVW_MODULE_CIMG_API

#include <inviwo/core/datastructures/image/imagetypes.h>  // for SwizzleMask
#include <inviwo/core/util/formats.h>                     // for DataFormatId, DataFormatBase (p...
#include <inviwo/core/util/glmvec.h>                      // for uvec2, size3_t, dvec2

#include <memory>                                         // for unique_ptr
#include <string>                                         // for string
#include <string_view>                                    // for string_view
#include <vector>                                         // for vector

namespace inviwo {
class Layer;
class LayerRAM;

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

enum class TIFFResolutionUnit { None, Inch, Centimeter };

struct IVW_MODULE_CIMG_API TIFFHeader {
    const DataFormatBase* format;
    size3_t dimensions;
    dvec2 resolution;
    TIFFResolutionUnit resolutionUnit;
    SwizzleMask swizzleMask;
};

IVW_MODULE_CIMG_API TIFFHeader getTIFFHeader(std::string_view filename);

/**
 * Loads layer data from a specified filePath.
 **/
IVW_MODULE_CIMG_API void* loadLayerData(void* dst, std::string_view filePath, uvec2& out_dim,
                                        DataFormatId& formatId, bool rescaleToDim = false);

/**
 * Loads volume data from a specified filePath.
 **/
IVW_MODULE_CIMG_API void* loadVolumeData(void* dst, std::string_view filePath, size3_t& out_dim,
                                         DataFormatId& formatId);

/**
 * Saves an layer of an image to a specified filename.
 * @param filePath the path including filename and extension, which is used to determine the image
 * format
 * @param inputImage specifies the image that is to be saved.
 */
IVW_MODULE_CIMG_API void saveLayer(std::string_view filePath, const Layer* inputImage);

/**
 * Saves an layer of an unsigned char buffer.
 * @param extension  specifies the output image format
 * @param inputImage specifies the image that is to be saved.
 */
IVW_MODULE_CIMG_API std::unique_ptr<std::vector<unsigned char>> saveLayerToBuffer(
    std::string_view extension, const Layer* inputImage);

/**
 * Load TIFF image data.
 * \see TIFFLayerReader
 * \see getTIFFHeader
 */
void* loadTIFFLayerData(void* dst, std::string_view filePath, TIFFHeader header,
                        bool rescaleToDim = false);

/**
 * Load TIFF stack as volume.
 * \see TIFFStackVolumeRAMLoader
 * \see getTIFFHeader
 */
void* loadTIFFVolumeData(void* dst, std::string_view filePath, TIFFHeader header);

/**
 * \brief Rescales Layer of given image data
 *
 * @param inputLayer image data that needs to be rescaled
 * @param dst_dim is destination dimensions
 * @return rescaled raw data
 */
IVW_MODULE_CIMG_API void* rescaleLayer(const Layer* inputLayer, uvec2 dst_dim);

/**
 * \brief Rescales LayerRAM representation uses FILTER_BILINEAR by default.
 *
 * @param layerRam representation that needs rescaling
 * @param dst_dim is destination dimensions
 * @return rescaled raw data
 */
IVW_MODULE_CIMG_API void* rescaleLayerRAM(const LayerRAM* layerRam, uvec2 dst_dim);

IVW_MODULE_CIMG_API bool rescaleLayerRamToLayerRam(const LayerRAM* source, LayerRAM* target);

IVW_MODULE_CIMG_API std::string getLibJPGVersion();
IVW_MODULE_CIMG_API std::string getOpenEXRVersion();

}  // namespace cimgutil

}  // namespace inviwo
