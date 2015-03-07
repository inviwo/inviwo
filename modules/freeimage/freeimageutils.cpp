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

#include <modules/freeimage/freeimageutils.h>
#include <algorithm>

bool FreeImageUtils::loader_initialized = false;

inline DataFormatEnums::Id getDataFormatFromBitmap(FIBITMAP* bitmap) {
    FREE_IMAGE_TYPE type = FreeImage_GetImageType(bitmap);
    unsigned int bpp = FreeImage_GetBPP(bitmap);

    switch (type) {
        case FIT_UNKNOWN:
            break;

        case FIT_BITMAP:
            switch (bpp) {
                case 1:
                case 4:
                case 8:
                    return inviwo::DataFormatEnums::UINT8;

                case 16:
                    return inviwo::DataFormatEnums::Vec2UINT8;

                case 24:
                case 32:
                    return inviwo::DataFormatEnums::Vec4UINT8;
            }

            break;

        case FIT_UINT16:
            return inviwo::DataFormatEnums::UINT16;

        case FIT_INT16:
            return inviwo::DataFormatEnums::INT16;

        case FIT_UINT32:
            return inviwo::DataFormatEnums::UINT32;

        case FIT_INT32:
            return inviwo::DataFormatEnums::INT32;

        case FIT_FLOAT:
            return inviwo::DataFormatEnums::FLOAT32;

        case FIT_DOUBLE:
            return inviwo::DataFormatEnums::FLOAT64;

        case FIT_COMPLEX:
            return inviwo::DataFormatEnums::Vec2FLOAT64;

        case FIT_RGB16:
            return inviwo::DataFormatEnums::Vec3UINT16;

        case FIT_RGBA16:
            return inviwo::DataFormatEnums::Vec4UINT16;

        case FIT_RGBF:
            return inviwo::DataFormatEnums::Vec3FLOAT32;

        case FIT_RGBAF:
            return inviwo::DataFormatEnums::Vec4FLOAT32;

        default:
            break;
    }

    return inviwo::DataFormatEnums::NOT_SPECIALIZED;
}

inline FREE_IMAGE_TYPE getFreeImageFormatFromDataFormat(inviwo::DataFormatEnums::Id formatId) {
    switch (formatId) {
        case inviwo::DataFormatEnums::NOT_SPECIALIZED:
            break;

        case inviwo::DataFormatEnums::UINT8:
        case inviwo::DataFormatEnums::Vec2UINT8:
        case inviwo::DataFormatEnums::Vec3UINT8:
        case inviwo::DataFormatEnums::Vec4UINT8:
            return FIT_BITMAP;

        case inviwo::DataFormatEnums::UINT16:
        case inviwo::DataFormatEnums::Vec2UINT16:
            return FIT_UINT16;

        case inviwo::DataFormatEnums::INT16:
        case inviwo::DataFormatEnums::Vec2INT16:
        case inviwo::DataFormatEnums::Vec3INT16:
        case inviwo::DataFormatEnums::Vec4INT16:
            return FIT_INT16;

        case inviwo::DataFormatEnums::UINT32:
        case inviwo::DataFormatEnums::Vec2UINT32:
        case inviwo::DataFormatEnums::Vec3UINT32:
        case inviwo::DataFormatEnums::Vec4UINT32:
            return FIT_UINT32;

        case inviwo::DataFormatEnums::INT32:
        case inviwo::DataFormatEnums::Vec2INT32:
        case inviwo::DataFormatEnums::Vec3INT32:
        case inviwo::DataFormatEnums::Vec4INT32:
            return FIT_INT32;

        case inviwo::DataFormatEnums::FLOAT32:
        case inviwo::DataFormatEnums::Vec2FLOAT32:
            return FIT_FLOAT;

        case inviwo::DataFormatEnums::FLOAT64:
        case inviwo::DataFormatEnums::Vec3FLOAT64:
        case inviwo::DataFormatEnums::Vec4FLOAT64:
            return FIT_DOUBLE;

        case inviwo::DataFormatEnums::Vec2FLOAT64:
            return FIT_COMPLEX;

        case inviwo::DataFormatEnums::Vec3UINT16:
            return FIT_RGB16;

        case inviwo::DataFormatEnums::Vec4UINT16:
            return FIT_RGBA16;

        case inviwo::DataFormatEnums::Vec3FLOAT32:
            return FIT_RGBF;

        case inviwo::DataFormatEnums::Vec4FLOAT32:
            return FIT_RGBAF;

        default:
            break;
    }

    return FIT_UNKNOWN;
}

void FreeImageUtils::saveLayer(const char* filename, const Layer* inputLayer) {
    initLoader();
    FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFIFFromFilename(filename);

    if (imageFormat != FIF_UNKNOWN && inputLayer != NULL) {
        const LayerRAM* imageRam = inputLayer->getRepresentation<LayerRAM>();
        assert(imageRam != NULL);
        FIBITMAP* bitmap = createBitmapFromData(imageRam, false);
        BOOL saved = 0;

        if (imageFormat == FIF_JPEG) {
            FIBITMAP* bitmapJPG = FreeImage_ConvertTo24Bits(bitmap);
            saved = FreeImage_Save(imageFormat, bitmapJPG, filename, 100);
            FreeImage_Unload(bitmapJPG);
        } else if (imageFormat == FIF_PNG) {
            FIBITMAP* bitmapPNG = FreeImage_ConvertTo32Bits(bitmap);
            saved = FreeImage_Save(imageFormat, bitmapPNG, filename);
            FreeImage_Unload(bitmapPNG);
        } else
            saved = FreeImage_Save(imageFormat, bitmap, filename,
                                   static_cast<int>(imageRam->getDataFormat()->getSize()));

        if (saved == 0) {
            LogErrorCustom("ImageIO", "Image layer could not be saved to " << filename);
        } else {
            LogInfoCustom("ImageIO", "Image layer saved to " << filename);
        }

        FreeImage_Unload(bitmap);
    } else {
        if (inputLayer == NULL) {
            LogErrorCustom("ImageIO", "Cannot save NULL image.");
        } else {
            // Unknown file ending
            LogErrorCustom("ImageIO", "Unknown file ending.");
        }
    }
}

bool FreeImageUtils::readInImage(std::string filename, FIBITMAP** bitmap) {
    const char* file_name_char = (char*)(filename.c_str());
    FREE_IMAGE_FORMAT imageFormat = FIF_UNKNOWN;
    //Get file format of input file
    imageFormat = FreeImage_GetFileType(file_name_char, 10);

    if (imageFormat == FIF_UNKNOWN) {
        imageFormat = FreeImage_GetFIFFromFilename(file_name_char);

        //Raw image files conflicting with raw 3d volumes.
        if (imageFormat == FIF_RAW)
            return false;
    }

    //Load image if format is supported
    if (imageFormat != FIF_UNKNOWN)
        *bitmap = FreeImage_Load(imageFormat, file_name_char, 0);

    //Return if format was found.
    return (imageFormat != FIF_UNKNOWN);
}

bool FreeImageUtils::isValidImageFile(std::string filename) {
    initLoader();
    const char* file_name_char = (char*)(filename.c_str());
    FREE_IMAGE_FORMAT imageFormat = FIF_UNKNOWN;
    //Get file format of input file
    imageFormat = FreeImage_GetFileType(file_name_char, 10);

    if (imageFormat == FIF_UNKNOWN) {
        imageFormat = FreeImage_GetFIFFromFilename(file_name_char);

        //Raw image files conflicting with raw 3d volumes.
        if (imageFormat == FIF_RAW)
            return false;
    }

    //Return if format was found.
    return (imageFormat != FIF_UNKNOWN);
}

void* FreeImageUtils::loadImageToData(void* data, std::string filename, uvec2& out_dim,
                                      inviwo::DataFormatEnums::Id& out_format) {
    initLoader();
    FIBITMAP* bitmap = 0;
    void* outData = data;
    out_format = DataFormatEnums::NOT_SPECIALIZED;

    if (readInImage(filename, &bitmap)) {
        unsigned int width = FreeImage_GetWidth(bitmap);
        unsigned int height = FreeImage_GetHeight(bitmap);
        out_dim = uvec2(width, height);
        out_format = getDataFormatFromBitmap(bitmap);

        switch (out_format) {
            case inviwo::DataFormatEnums::NOT_SPECIALIZED:
                LogErrorCustom("loadImageToData", "Invalid format");
                break;
#define DataFormatIdMacro(i)                                                            \
    case inviwo::DataFormatEnums::i:                                                    \
        outData = fiBitmapToDataArray<Data##i::type>(data, bitmap, 8 * Data##i::size(), \
                                                     Data##i::components());            \
        break;
#include <inviwo/core/util/formatsdefinefunc.h>

            default:
                LogErrorCustom("loadImageToData", "Invalid format or not implemented");
                break;
        }
    }

    FreeImage_Unload(bitmap);
    return outData;
}

void* FreeImageUtils::loadImageToDataAndRescale(void* data, std::string filename, uvec2 dst_dim,
                                                inviwo::DataFormatEnums::Id& out_format) {
    initLoader();
    FIBITMAP* bitmap = 0;
    void* outData = data;
    out_format = inviwo::DataFormatEnums::NOT_SPECIALIZED;

    if (readInImage(filename, &bitmap)) {
        out_format = getDataFormatFromBitmap(bitmap);

        switch (out_format) {
            case inviwo::DataFormatEnums::NOT_SPECIALIZED:
                LogErrorCustom("loadImageToDataAndRescale", "Invalid format");
                break;
#define DataFormatIdMacro(i)                                                    \
    case inviwo::DataFormatEnums::i:                                            \
        outData = fiBitmapToDataArrayAndRescale<Data##i::type>(                 \
            data, bitmap, dst_dim, 8 * Data##i::size(), Data##i::components()); \
        break;
#include <inviwo/core/util/formatsdefinefunc.h>

            default:
                LogErrorCustom("loadImageToDataAndRescale", "Invalid format or not implemented");
                break;
        }
    }

    FreeImage_Unload(bitmap);
    return outData;
}

void* FreeImageUtils::rescaleLayer(const Layer* inputLayer, uvec2 dst_dim) {
    const LayerRAM* layerRam = inputLayer->getRepresentation<LayerRAM>();
    return rescaleLayerRAM(layerRam, dst_dim);
}

void* FreeImageUtils::rescaleLayerRAM(const LayerRAM* srcLayerRam, uvec2 dst_dim) {
    ivwAssert(srcLayerRam != NULL, "LayerRAM representation does not exist.");
    initLoader();
    void* rawData = NULL;
    FIBITMAP* bitmap = NULL;
    FREE_IMAGE_TYPE formatType = getFreeImageFormatFromDataFormat(srcLayerRam->getDataFormatId());

    ivwAssert(srcLayerRam->getDimensions() != uvec2(0),
              "Trying to rescale layer with zero dimensions.");

    switch (srcLayerRam->getDataFormatId()) {
        case inviwo::DataFormatEnums::NOT_SPECIALIZED:
            LogErrorCustom("rescaleLayerRAM", "Invalid format");
            rawData = NULL;
            break;
#define DataFormatIdMacro(i)                                                          \
    case inviwo::DataFormatEnums::i:                                                  \
        bitmap = handleBitmapCreations<Data##i::type>(                                \
            static_cast<const Data##i::type*>(srcLayerRam->getData()), formatType,    \
            srcLayerRam->getDimensions(), 8 * Data##i::size(), Data##i::components(), \
            srcLayerRam->getDataFormat());                                            \
        rawData = fiBitmapToDataArrayAndRescale<Data##i::type>(                       \
            NULL, bitmap, dst_dim, 8 * Data##i::size(), Data##i::components());       \
        break;
#include <inviwo/core/util/formatsdefinefunc.h>

        default:
            LogErrorCustom("rescaleLayerRAM", "Invalid format or not implemented");
            rawData = NULL;
            break;
    }

    ivwAssert(rawData != NULL, "Unable to rescale image ram representation.");

    FreeImage_Unload(bitmap);
    return rawData;
}

void FreeImageUtils::switchChannels(FIBITMAP* bitmap, uvec2 dim, int channels) {
    if (bitmap && channels > 2) {
        FREE_IMAGE_TYPE type = FreeImage_GetImageType(bitmap);
        if (type == FIT_BITMAP) {
            unsigned int c = static_cast<unsigned int>(channels);
            BYTE* result = FreeImage_GetBits(bitmap);

            for (unsigned int i = 0; i < dim.x * dim.y; i++) {
                std::swap(result[i * c], result[i * c + 2]);
            }
        }
    }
}

FIBITMAP* FreeImageUtils::allocateBitmap(FREE_IMAGE_TYPE type, uvec2 dim, size_t bitsPerPixel,
                                         int channels) {
    unsigned int rMask = FI_RGBA_RED_MASK;
    unsigned int gMask = FI_RGBA_GREEN_MASK;
    unsigned int bMask = FI_RGBA_BLUE_MASK;

    if (channels == 2)
        bMask = 0;
    else if (channels == 1) {
        gMask = 0;
        bMask = 0;
    } else if (channels == 0) {
        rMask = 0;
        gMask = 0;
        bMask = 0;
    }

    return FreeImage_AllocateT(type, static_cast<int>(dim.x), static_cast<int>(dim.y),
                               static_cast<int>(bitsPerPixel), rMask, gMask, bMask);
}

template <typename T>
FIBITMAP* FreeImageUtils::createBitmapFromData(const T* data, FREE_IMAGE_TYPE type, uvec2 dim,
                                               size_t bitsPerPixel, int channels,
                                               const DataFormatBase* format, bool noScaling) {
    FIBITMAP* dib = allocateBitmap(type, dim, bitsPerPixel, channels);
    if (!dib) return NULL;
    unsigned int bytespp = FreeImage_GetLine(dib) / FreeImage_GetWidth(dib);
    T* bits = (T*)FreeImage_GetBits(dib);

    // Scale normalized float value to from 0 - 1 to 0  - 255
    if (!noScaling && type == FIT_FLOAT) {
        T value;
        format->doubleToValue(255.0, &value);

        for (unsigned int i = 0; i < dim.x * dim.y; i++) bits[i] = data[i] * value;

        FIBITMAP* dibConvert = FreeImage_ConvertToStandardType(dib);
        return dibConvert;
    } else
        memcpy(bits, data, dim.x * dim.y * bytespp);

    return dib;
}

template <typename T>
FIBITMAP* FreeImageUtils::handleBitmapCreations(const T* data, FREE_IMAGE_TYPE type, uvec2 dim,
                                                size_t bitsPerPixel, int channels,
                                                const DataFormatBase* format, bool noScaling) {
    FIBITMAP* bitmap =
        createBitmapFromData<T>(data, type, dim, bitsPerPixel, channels, format, noScaling);
    switchChannels(bitmap, dim, channels);
    return bitmap;
}

FIBITMAP* FreeImageUtils::createBitmapFromData(const LayerRAM* inputLayer, bool noScaling) {
    initLoader();
    FREE_IMAGE_TYPE formatType = getFreeImageFormatFromDataFormat(inputLayer->getDataFormatId());

    switch (inputLayer->getDataFormatId()) {
        case inviwo::DataFormatEnums::NOT_SPECIALIZED:
            LogErrorCustom("createBitmapFromData", "Invalid format");
            return NULL;
#define DataFormatIdMacro(i)                                                         \
    case inviwo::DataFormatEnums::i:                                                 \
        return handleBitmapCreations<Data##i::type>(                                 \
            static_cast<const Data##i::type*>(inputLayer->getData()), formatType,    \
            inputLayer->getDimensions(), 8 * Data##i::size(), Data##i::components(), \
            inputLayer->getDataFormat(), noScaling);
#include <inviwo/core/util/formatsdefinefunc.h>

        default:
            LogErrorCustom("createBitmapFromData", "Invalid format or not implemented");
            break;
    }

    return NULL;
}

void FreeImageUtils::copyBitmapToData(FIBITMAP* bitmap, LayerRAM* outImage) {
    initLoader();
    switch (outImage->getDataFormat()->getId()) {
        case inviwo::DataFormatEnums::NOT_SPECIALIZED:
            LogErrorCustom("loadImageToData", "Invalid format");
            break;
#define DataFormatIdMacro(i)                                                                 \
    case inviwo::DataFormatEnums::i:                                                         \
        fiBitmapToDataArray<Data##i::type>(outImage->getData(), bitmap, 8 * Data##i::size(), \
                                           Data##i::components());                           \
        break;
#include <inviwo/core/util/formatsdefinefunc.h>

        default:
            LogErrorCustom("loadImageToData", "Invalid format or not implemented");
            break;
    }
}

template <typename T>
void* FreeImageUtils::fiBitmapToDataArray(void* dst, FIBITMAP* bitmap, size_t bitsPerPixel,
                                          int channels) {
    unsigned int width = FreeImage_GetWidth(bitmap);
    unsigned int height = FreeImage_GetHeight(bitmap);
    FREE_IMAGE_TYPE type = FreeImage_GetImageType(bitmap);
    uvec2 dim(width, height);
    FIBITMAP* bitmapNEW = allocateBitmap(type, dim, bitsPerPixel, channels);
    if (!bitmapNEW) return NULL;
    FreeImage_Paste(bitmapNEW, bitmap, 0, 0, 256);
    switchChannels(bitmapNEW, dim, channels);
    void* pixelValues = static_cast<void*>(FreeImage_GetBits(bitmapNEW));

    if (!dst) {
        T* dstAlloc = new T[dim.x * dim.y];
        dst = static_cast<void*>(dstAlloc);
    }

    memcpy(dst, pixelValues, dim.x * dim.y * sizeof(T));
    FreeImage_Unload(bitmapNEW);
    return dst;
}

template <typename T>
void* FreeImageUtils::fiBitmapToDataArrayAndRescale(void* dst, FIBITMAP* bitmap, uvec2 dst_dim,
                                                    size_t bitsPerPixel, int channels) {
    int width = FreeImage_GetWidth(bitmap);
    int height = FreeImage_GetHeight(bitmap);
    uvec2 dim(width, height);

    // No rescale needed if output is of same size as input
    if (dim == dst_dim) return fiBitmapToDataArray<T>(dst, bitmap, bitsPerPixel, channels);

    // We want to respect aspect ratio, so check if we need to perform some alteration for that
    float ratioSource = static_cast<float>(dim.x) / static_cast<float>(dim.y);
    float ratioTarget = static_cast<float>(dst_dim.x) / static_cast<float>(dst_dim.y);

    // Determine size of our the image we want to scale from
    uvec2 dimTmp = dim;
    int pasteLeft = 0;
    int pasteTop = 0;
    if (ratioTarget > ratioSource) {
        dimTmp.x = static_cast<glm::u32>(
            glm::ceil(static_cast<float>(dim.x) * (ratioTarget / ratioSource)));
        pasteLeft = static_cast<int>((dimTmp.x - dim.x) / 2);
    } else if (ratioTarget < ratioSource) {
        dimTmp.y = static_cast<glm::u32>(
            glm::ceil(static_cast<float>(dim.y) * (ratioSource / ratioTarget)));
        pasteTop = static_cast<int>((dimTmp.y - dim.y) / 2);
    }

    // Allocate our tmp bitmap
    FREE_IMAGE_TYPE type = FreeImage_GetImageType(bitmap);
    FIBITMAP* bitmapTmp = allocateBitmap(type, dimTmp, bitsPerPixel, channels);
    if (!bitmapTmp) return NULL;
    // Paste source into our tmp, to be used for scaling
    FreeImage_Paste(bitmapTmp, bitmap, pasteLeft, pasteTop, 256);

    // Rescale to proper dimension
    FIBITMAP* bitmapRescaled = FreeImage_Rescale(bitmapTmp, static_cast<int>(dst_dim.x),
                                                 static_cast<int>(dst_dim.y), FILTER_BILINEAR);
    FreeImage_Unload(bitmapTmp);
    switchChannels(bitmapRescaled, dst_dim, channels);
    unsigned char* pixelValues = static_cast<unsigned char*>(FreeImage_GetBits(bitmapRescaled));

    if (!dst) {
        T* dstAlloc = new T[dst_dim.x * dst_dim.y];
        dst = static_cast<void*>(dstAlloc);
    }

    memcpy(dst, pixelValues, dst_dim.x * dst_dim.y * sizeof(T));
    FreeImage_Unload(bitmapRescaled);
    return dst;
}

void FreeImageUtils::initLoader() {
    if (!loader_initialized) {
        loader_initialized = true;
        FreeImage_Initialise(1);
    }
}

std::vector<unsigned char>* FreeImageUtils::saveLayerToBuffer(const char* type,
                                                              const Layer* inputLayer) {
    initLoader();
    FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFIFFromFilename(type);

    if (imageFormat != FIF_UNKNOWN && inputLayer != NULL) {
        const LayerRAM* imageRam = inputLayer->getRepresentation<LayerRAM>();
        assert(imageRam != NULL);
        FIBITMAP* bitmap = createBitmapFromData(imageRam, false);
        BOOL saved = 0;

        FIMEMORY* mem = FreeImage_OpenMemory();

        saved =
            FreeImage_SaveToMemory(imageFormat, bitmap, mem,
                                   static_cast<int>(8 * imageRam->getDataFormat()->getSize()));

        DWORD size_in_bytes = 0;
        BYTE* mem_buffer = NULL;
        if (!FreeImage_AcquireMemory(mem, &mem_buffer, &size_in_bytes))

            if (saved == 0) {
                LogErrorCustom("ImageIO", "Image layer could not be saved");
            }

        std::vector<unsigned char>* data =
            new std::vector<BYTE>(&mem_buffer[0], &mem_buffer[size_in_bytes]);

        FreeImage_Unload(bitmap);
        FreeImage_CloseMemory(mem);

        return data;
    } else {
        // Unknown file ending
        LogErrorCustom("ImageIO", "Unknown file ending.");
    }
    return NULL;
}
