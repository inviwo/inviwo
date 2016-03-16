/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_CIMGUTILS_H
#define IVW_CIMGUTILS_H

#include <modules/cimg/cimgmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>

namespace inviwo {

namespace cimgutil {
    enum class InterpolationType : int {
        RawMemory = -1,       // raw memory resizing.
        NoInterpolation = 0,  // additional space is filled according to boundary_conditions.
        Nearset = 1,          // nearest - neighbor interpolation.
        moving = 2,           // moving average interpolation.
        Linear = 3,           // linear interpolation.
        Grid = 4,             // grid interpolation.
        Cubic = 5,            // cubic interpolation.
        Lanczos = 6           // lanczos interpolation.
    };

    /**
    * Loads layer data from a specified filePath.
    **/
    IVW_MODULE_CIMG_API void* loadLayerData(void* dst, const std::string& filePath, uvec2& out_dim,
                                            DataFormatId& formatId, bool rescaleToDim = false);

    /**
    * Loads volume data from a specified filePath.
    **/
    IVW_MODULE_CIMG_API void* loadVolumeData(void* dst, const std::string& filePath,
                                             size3_t& out_dim, DataFormatId& formatId);

    /**
    * Saves an layer of an image to a specified filename.
    * @param filePath the path including name to file that is to be stored.
    * @param inputImage specifies the image that is to be saved.
    **/
    IVW_MODULE_CIMG_API void saveLayer(const std::string& filePath, const Layer* inputImage);

    /**
    * Saves an layer of an buffer.
    * @param file type the requested file type, which can be altered by this method
    * @param inputImage specifies the image that is to be saved.
    **/
    IVW_MODULE_CIMG_API std::unique_ptr<std::vector<unsigned char>> saveLayerToBuffer(
        std::string& fileType, const Layer* inputImage);

    /**
     * \brief Rescales Layer of given image data
     *
     * @param Layer * inputImage image data that needs to be rescaled
     * @param uvec2 dst_dim is destination dimensions
     * @param void* rescaled raw data
     */
    IVW_MODULE_CIMG_API void* rescaleLayer(const Layer* inputLayer, uvec2 dst_dim);

    /**
     * \brief Rescales LayerRAM representation uses FILTER_BILINEAR by default.
     *
     * @param LayerRAM * imageRam representation that needs rescaling
     * @param uvec2 dst_dim is destination dimensions
     * @param void* rescaled raw data
     */
    IVW_MODULE_CIMG_API void* rescaleLayerRAM(const LayerRAM* layerRam, uvec2 dst_dim);

    IVW_MODULE_CIMG_API bool rescaleLayerRamToLayerRam(const LayerRAM* source, LayerRAM* target);

    } // namespace cimgutil

}  // namespace inviwo

#endif  // IVW_CIMGUTILS_H