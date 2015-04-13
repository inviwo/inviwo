/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_FREEIMAGEUTILS_H
#define IVW_FREEIMAGEUTILS_H

#include <modules/freeimage/freeimagemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <modules/freeimage/ext/freeimage/FreeImage.h>

using namespace inviwo;

class IVW_MODULE_FREEIMAGE_API FreeImageUtils {
public:
    FreeImageUtils(){};
    ~FreeImageUtils() { FreeImage_DeInitialise(); }
    /**
    * Loads an image through bitmap.
    * @param void* will point to the raw data
    * @param filename is the file that is to be loaded
    * @return DataFormatEnums::Id the data format
    */
    static void* loadImageToData(void* data, std::string filename, uvec2& out_dim,
                                 DataFormatEnums::Id& out_format);

    /**
     * \brief Loads an image through bitmap and rescale
     *
     * @param void* will point to the raw data
     * @param std::string filename is the file that is to be loaded
     * @param uvec2 dst_dim is destination dimensions
     * @return DataFormatEnums::Id the data format
     */
    static void* loadImageToDataAndRescale(void* data, std::string filename, uvec2 dist_dim,
                                           DataFormatEnums::Id& out_format);

    /**
    * Saves an layer of an image to a specified filename.
    * @param filename the path including name to file that is to be stored.
    * @param inputImage specifies the image that is to be saved.
    **/
    static void saveLayer(const char* filename, const Layer* inputImage);

    /**
    * Saves an layer of an image to a specified filename.
    * @param filename the path including name to file that is to be stored.
    * @param inputImage specifies the image that is to be saved.
    **/
    static std::vector<unsigned char>* saveLayerToBuffer(const char* type, const Layer* inputImage);

    /**
     * \brief Rescales Layer of given image data
     *
     * @param Layer * inputImage image data that needs to be rescaled
     * @param uvec2 dst_dim is destination dimensions
     * @param void* rescaled raw data
     */
    static void* rescaleLayer(const Layer* inputLayer, uvec2 dst_dim);

    /**
     * \brief Rescales LayerRAM representation uses FILTER_BILINEAR by default.
     *
     * @param LayerRAM * imageRam representation that needs rescaling
     * @param uvec2 dst_dim is destination dimensions
     * @param void* rescaled raw data
     */
    static void* rescaleLayerRAM(const LayerRAM* layerRam, uvec2 dst_dim);

    static bool isValidImageFile(std::string fileExtension);

    /**
    * Create bitmap from image.
    */
    static FIBITMAP* createBitmapFromData(const LayerRAM* inputImage, bool noScaling = true);

    /**
    * Copy bitmap to layer
    */
    static void copyBitmapToData(FIBITMAP* bitmap, LayerRAM* outImage);

private:
    /**
    * Internal function to load a image
    * @param filename the name of the file to be loaded
    * @param bitmap the bitmap to store the loaded image.
    * @return if the load was successful.
    */
    static bool readInImage(std::string filename, FIBITMAP** bitmap);

    /**
    * Initializes freeimage if needed.
    **/
    static void initLoader();

    /**
    * Converts image to byte array.
    */
    static FIBITMAP* convertToByte(const LayerRAM* inputImage, uvec2 dim, size_t bitsPerPixel);

    static FIBITMAP* allocateBitmap(FREE_IMAGE_TYPE type, uvec2 dim, size_t bitsPerPixel,
                                    int channels);

    template <typename T>
    static FIBITMAP* handleBitmapCreations(const T* data, FREE_IMAGE_TYPE type, uvec2 dim,
                                           size_t bitsPerPixel, int channels,
                                           const DataFormatBase* format, bool noScaling = true);

    template <typename T>
    static FIBITMAP* createBitmapFromData(const T* data, FREE_IMAGE_TYPE type, uvec2 dim,
                                          size_t bitsPerPixel, int channels,
                                          const DataFormatBase* format, bool noScaling = true);

    /**
    * Switch red and blue channels in the bitmap.
    */
    static void switchChannels(FIBITMAP* bitmap, uvec2 dim, int channels);

    /**
    * Converts an image from freeimage format to regular int.
    **/
    template <typename T>
    static void* fiBitmapToDataArray(void* dst, FIBITMAP* bitmap, size_t bitsPerPixel,
                                     int channels);

    /**
     * \brief fits the bitmap into data array which is readable by representations such as LayerRAM
     * that uses FILTER_BILINEAR
     */
    template <typename T>
    static void* fiBitmapToDataArrayAndRescale(void* dst, FIBITMAP* bitmap, uvec2 dst_dim,
                                               size_t bitsPerPixel, int channels);

    static bool loader_initialized;
};
#endif  // IVW_FREEIMAGEUTILS_H