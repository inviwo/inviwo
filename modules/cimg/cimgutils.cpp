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

#include <modules/cimg/cimgutils.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/util/filesystem.h>
#include <algorithm>

#include <warn/push>
#include <warn/ignore/all>
#include <modules/cimg/ext/cimg/CImg.h>
#include <warn/pop>

#include <warn/push>
#include <warn/ignore/switch-enum>
#include <warn/ignore/conversion>
#if (_MSC_VER)
# pragma warning(disable: 4146)
# pragma warning(disable: 4197)
# pragma warning(disable: 4297)
# pragma warning(disable: 4267)
# pragma warning(disable: 4293)
#endif



// Added in Cimg.h below struct type<float>...
/*#include <limits>
#include <half/half.hpp>

    template<> struct type < half_float::half > {
        static const char* string() { static const char *const s = "half"; return s; }
        static bool is_float() { return true; }
        static bool is_inf(const half_float::half val) {
#ifdef isinf
            return (bool)isinf(val);
#else
            return !is_nan(val) && (val<cimg::type<half_float::half>::min() || val>cimg::type<half_float::half>::max());
#endif
        }
        static bool is_nan(const half_float::half val) {
#ifdef isnan
            return (bool)isnan(val);
#else
            return !(val == val);
#endif
        }
        static half_float::half min() { return std::numeric_limits<half_float::half>::min(); }
        static half_float::half max() { return std::numeric_limits<half_float::half>::max(); }
        static half_float::half inf() { return (half_float::half)cimg::type<double>::inf(); }
        static half_float::half nan() { return (half_float::half)cimg::type<double>::nan(); }
        static half_float::half cut(const double val) { return val<(double)min() ? min() : val>(double)max() ? max() : (half_float::half)val; }
        static const char* format() { return "%.16g"; }
        static double format(const half_float::half val) { return (double)val; }
    };
*/

using namespace cimg_library;

namespace inviwo {

std::unordered_map<std::string, DataFormatEnums::Id> extToBaseTypeMap_ = {
          { "png", DataFormatEnums::UINT8 }
        , { "jpg", DataFormatEnums::UINT8 }
        , { "jpeg", DataFormatEnums::UINT8 }
        , { "bmp", DataFormatEnums::UINT8 }
        , { "exr", DataFormatEnums::FLOAT32 }
        , { "hdr", DataFormatEnums::FLOAT32 }
};

////////////////////// Templates ///////////////////////////////////////////////////

// Single channel images
template <typename T>
struct CImgToIvwConvert {
    static void* convert(void* dst, CImg<T>* img) {
        //Inviwo store pixels interleaved (RGBRGBRGB), CImg stores pixels in a planer format (RRRRGGGGBBBB). 
        //Permute from planer to interleaved format, does we need to specify cxyz as input instead of xyzc
        if (img->spectrum() > 1){
            img->permute_axes("cxyz");
        }

        if (!dst) {
            T* dstAlloc = new T[img->size()];
            dst = static_cast<void*>(dstAlloc);
        }
        const void* src = static_cast<const void*>(img->data());

        std::memcpy(dst, src, img->size()*sizeof(T));

        return dst;
    }
};

// Single channel images
template <typename T>
struct LayerToCImg {
    static CImg<T>* convert(const LayerRAM* inputLayerRAM, bool permute = true) {
        //Single channel means we can do xyzc, as no permutation is needed
        CImg<T>* img = new CImg<T>(static_cast<const T*>(inputLayerRAM->getData()),
            inputLayerRAM->getDimensions().x, inputLayerRAM->getDimensions().y, 1, 1, false);

        return img;
    }
};

// Multiple channel images
template <typename T, template <typename, glm::precision> class G>
struct LayerToCImg<G<T, glm::defaultp>> {
    static CImg<T>* convert(const LayerRAM* inputLayerRAM, bool permute = true) {
        const DataFormatBase* dataFormat = inputLayerRAM->getDataFormat();
        const G<T, glm::defaultp>* typedDataPtr = static_cast<const G<T, glm::defaultp>*>(inputLayerRAM->getData());

        //Inviwo store pixels interleaved (RGBRGBRGB), CImg stores pixels in a planer format (RRRRGGGGBBBB). 
        //Permute from interleaved to planer format, does we need to specify yzcx as input instead of cxyz
        CImg<T>* img = new CImg<T>(glm::value_ptr(*typedDataPtr),
            dataFormat->getComponents(), inputLayerRAM->getDimensions().x, inputLayerRAM->getDimensions().y, 1, false);
        
        if (permute)
            img->permute_axes("yzcx");

        return img;
    }
};

struct CImgNormalizedLayerDispatcher {
    using type = std::vector<unsigned char>*;
    template <typename T>
    std::vector<unsigned char>* dispatch(const LayerRAM* inputLayer) {
        CImg<typename T::primitive>* img = LayerToCImg<typename T::type>::convert(inputLayer, false);

        CImg<unsigned char> normalizedImg = img->get_normalize(0, 255);

        normalizedImg.mirror('z');

        std::vector<unsigned char>* data =
            new std::vector<unsigned char>(&normalizedImg[0], &normalizedImg[normalizedImg.size()]);

        delete img;

        return data;
    }
};

struct CImgLoadLayerDispatcher {
    using type = void*;
    template <typename T>
    void* dispatch(void* dst, const char* filePath, uvec2& dimensions, DataFormatEnums::Id& formatId, const DataFormatBase* dataFormat, bool rescaleToDim) {
        CImg<typename T::primitive> img(filePath);

        size_t components = static_cast<size_t>(img.spectrum());

        if (rescaleToDim) {
            img.resize(dimensions.x, dimensions.y);
        }
        else{
            dimensions = uvec2(img.width(), img.height());
        }

        const DataFormatBase* loadedDataFormat = DataFormatBase::get(dataFormat->getNumericType(), components, sizeof(typename T::primitive)*8);
        if (loadedDataFormat)
            formatId = loadedDataFormat->getId();
        else
            throw Exception("CImgLoadLayerDispatcher, could not find proper data type");

        //Image is up-side-down
        img.mirror('y');

        return CImgToIvwConvert<typename T::primitive>::convert(dst, &img);
    }
};

struct CImgSaveLayerDispatcher {
    using type = void;
    template <typename T>
    void dispatch(const char* filePath, const LayerRAM* inputLayer) {
        CImg<typename T::primitive>* img = LayerToCImg<typename T::type>::convert(inputLayer);

        //Should normalize based on output format i.e. PNG/JPG is 0-255, HDR different.
        const DataFormatBase* outFormat = DataFLOAT32::get();
        std::string fileExtension = filesystem::getFileExtension(filePath);
        if (extToBaseTypeMap_.find(fileExtension) != extToBaseTypeMap_.end()) {
            outFormat = DataFormatBase::get(extToBaseTypeMap_[fileExtension]);
        }

        //Image is up-side-down
        img->mirror('y');

        img->normalize(static_cast<typename T::primitive>(outFormat->getMin()), static_cast<typename T::primitive>(outFormat->getMax()));
        img->save(filePath);
        delete img;
    }
};

struct CImgRescaleLayerDispatcher {
    using type = void*;
    template <typename T>
    void* dispatch(const LayerRAM* inputLayerRAM, uvec2 dst_dim) {
        CImg<typename T::primitive>* img = LayerToCImg<typename T::type>::convert(inputLayerRAM);

        img->resize(dst_dim.x, dst_dim.y);

        return CImgToIvwConvert<typename T::primitive>::convert(nullptr, img);
    }
};

struct CImgLoadVolumeDispatcher {
    using type = void*;
    template <typename T>
    void* dispatch(void* dst, const char* filePath, size3_t& dimensions, DataFormatEnums::Id& formatId, const DataFormatBase* dataFormat) {
        CImg<typename T::primitive> img(filePath);

        size_t components = static_cast<size_t>(img.spectrum());
        dimensions = size3_t(img.width(), img.height(), img.depth());

        const DataFormatBase* loadedDataFormat = DataFormatBase::get(dataFormat->getNumericType(), components, sizeof(typename T::primitive) * 8);
        if (loadedDataFormat)
            formatId = loadedDataFormat->getId();
        else
            throw Exception("CImgLoadVolumeDispatcher, could not find proper data type");

        //Image is up-side-down
        img.mirror('y');

        return CImgToIvwConvert<typename T::primitive>::convert(dst, &img);
    }
};


////////////////////// CImgUtils ///////////////////////////////////////////////////

void* CImgUtils::loadLayerData(void* dst, const std::string& filePath, uvec2& dimensions,
    DataFormatEnums::Id& formatId, bool rescaleToDim){
    std::string fileExtension = filesystem::getFileExtension(filePath);
    if (extToBaseTypeMap_.find(fileExtension) != extToBaseTypeMap_.end()) {
        formatId = extToBaseTypeMap_[fileExtension];
    }
    else{
        formatId = DataFormatEnums::FLOAT32;
    }
    const DataFormatBase* dataFormat = DataFormatBase::get(formatId);

    CImgLoadLayerDispatcher disp;
    return dataFormat->dispatch(disp, dst, filePath.c_str(), dimensions, formatId, dataFormat, rescaleToDim);
}

void* CImgUtils::loadVolumeData(void* dst, const std::string& filePath, size3_t& dimensions,
    DataFormatEnums::Id& formatId){
    std::string fileExtension = filesystem::getFileExtension(filePath);
    if (extToBaseTypeMap_.find(fileExtension) != extToBaseTypeMap_.end()) {
        formatId = extToBaseTypeMap_[fileExtension];
    }
    else{
        formatId = DataFormatEnums::FLOAT32;
    }
    const DataFormatBase* dataFormat = DataFormatBase::get(formatId);

    CImgLoadVolumeDispatcher disp;
    return dataFormat->dispatch(disp, dst, filePath.c_str(), dimensions, formatId, dataFormat);
}

void CImgUtils::saveLayer(const std::string& filePath, const Layer* inputLayer) {
    CImgSaveLayerDispatcher disp;
    const LayerRAM* inputLayerRam = inputLayer->getRepresentation<LayerRAM>();
    inputLayer->getDataFormat()->dispatch(disp, filePath.c_str(), inputLayerRam);
}

std::vector<unsigned char>* CImgUtils::saveLayerToBuffer(std::string& fileType, const Layer* inputLayer) {
    CImgNormalizedLayerDispatcher disp;
    const LayerRAM* inputLayerRam = inputLayer->getRepresentation<LayerRAM>();
    // Can only produce raw output
    fileType = "raw";
    return inputLayer->getDataFormat()->dispatch(disp, inputLayerRam);
}

void* CImgUtils::rescaleLayer(const Layer* inputLayer, uvec2 dst_dim) {
    const LayerRAM* layerRam = inputLayer->getRepresentation<LayerRAM>();
    return rescaleLayerRAM(layerRam, dst_dim);
}

void* CImgUtils::rescaleLayerRAM(const LayerRAM* srcLayerRam, uvec2 dst_dim) {
    CImgRescaleLayerDispatcher disp;
    return srcLayerRam->getDataFormat()->dispatch(disp, srcLayerRam, dst_dim);
}

}  // namespace

#include <warn/pop>

