/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2018 Inviwo Foundation
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
#include <modules/cimg/cimgsavebuffer.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datawriterexception.h>
#include <inviwo/core/io/datareaderexception.h>
#include <algorithm>
#include <limits>

#include <warn/push>
#include <warn/ignore/all>
#if (_MSC_VER)
#pragma warning(disable : 4146)
#pragma warning(disable : 4197)
#pragma warning(disable : 4293)
#pragma warning(disable : 4309)
#pragma warning(disable : 4319)
#pragma warning(disable : 4324)
#pragma warning(disable : 4456)
#pragma warning(disable : 4458)
#pragma warning(disable : 4611)
#endif
#include <CImg.h>
#include <warn/pop>

#include <warn/push>
#include <warn/ignore/switch-enum>
#include <warn/ignore/conversion>
#if (_MSC_VER)
#pragma warning(disable : 4297)
#pragma warning(disable : 4267)
#endif

// add CImg type specialization for half_float::half
namespace cimg_library {
namespace cimg {

template <>
struct type<half_float::half> {
    static const char* string() {
        static const char* const s = "half";
        return s;
    }
    static bool is_float() { return true; }
    static bool is_inf(const long double val) {
        return half_float::isinf(static_cast<half_float::half>(val));
    }
    static bool is_nan(const long double val) {
        return half_float::isnan(static_cast<half_float::half>(val));
    }
    static half_float::half min() { return std::numeric_limits<half_float::half>::lowest(); }
    static half_float::half max() { return std::numeric_limits<half_float::half>::max(); }
    static half_float::half inf() { return std::numeric_limits<half_float::half>::infinity(); }
    static half_float::half nan() { return std::numeric_limits<half_float::half>::quiet_NaN(); }
    static half_float::half cut(const double val) { return static_cast<half_float::half>(val); }
    static const char* format() { return "%.9g"; }
    static const char* format_s() { return "%g"; }
    static double format(const half_float::half val) { return static_cast<double>(val); }
};

template <>
struct superset<bool, half_float::half> {
    typedef half_float::half type;
};
template <>
struct superset<half_float::half, unsigned short> {
    typedef float type;
};
template <>
struct superset<half_float::half, short> {
    typedef float type;
};
template <>
struct superset<half_float::half, unsigned int> {
    typedef float type;
};
template <>
struct superset<half_float::half, int> {
    typedef float type;
};
template <>
struct superset<half_float::half, cimg_uint64> {
    typedef float type;
};
template <>
struct superset<half_float::half, cimg_int64> {
    typedef float type;
};
template <>
struct superset<half_float::half, float> {
    typedef float type;
};
template <>
struct superset<half_float::half, double> {
    typedef double type;
};

}  // namespace cimg
}  // namespace cimg_library

namespace inviwo {

namespace cimgutil {

std::unordered_map<std::string, DataFormatId> extToBaseTypeMap_ = {{"jpg", DataFormatId::UInt8},
                                                                   {"jpeg", DataFormatId::UInt8},
                                                                   {"bmp", DataFormatId::UInt8},
                                                                   {"exr", DataFormatId::Float32},
                                                                   {"hdr", DataFormatId::Float32}};

////////////////////// Templates ///////////////////////////////////////////////////

// Single channel images
template <typename T>
struct CImgToVoidConvert {
    static void* convert(void* dst, cimg_library::CImg<T>* img) {
        // Inviwo store pixels interleaved (RGBRGBRGB), CImg stores pixels in a planer format
        // (RRRRGGGGBBBB).
        // Permute from planer to interleaved format, does we need to specify cxyz as input instead
        // of xyzc
        if (img->spectrum() > 1) {
            img->permute_axes("cxyz");
        }

        if (!dst) {
            T* dstAlloc = new T[img->size()];
            dst = static_cast<void*>(dstAlloc);
        }
        const void* src = static_cast<const void*>(img->data());

        std::memcpy(dst, src, img->size() * sizeof(T));

        return dst;
    }
};

// Single channel images
template <typename T>
struct LayerToCImg {
    static std::unique_ptr<cimg_library::CImg<T>> convert(const LayerRAM* inputLayerRAM,
                                                          bool /*permute*/ = true) {
        // Single channel means we can do xyzc, as no permutation is needed
        auto img = util::make_unique<cimg_library::CImg<T>>(
            static_cast<const T*>(inputLayerRAM->getData()),
            static_cast<unsigned int>(inputLayerRAM->getDimensions().x),
            static_cast<unsigned int>(inputLayerRAM->getDimensions().y), 1, 1, false);

        return img;
    }
};

// Multiple channel images
template <glm::length_t L, typename T, glm::qualifier Q>
struct LayerToCImg<glm::vec<L, T, Q>> {
    static std::unique_ptr<cimg_library::CImg<T>> convert(const LayerRAM* inputLayerRAM,
                                                          bool permute = true) {
        auto dataFormat = inputLayerRAM->getDataFormat();
        auto typedDataPtr = static_cast<const glm::vec<L, T, Q>*>(inputLayerRAM->getData());

        // Inviwo store pixels interleaved (RGBRGBRGB), CImg stores pixels in a planer format
        // (RRRRGGGGBBBB).
        // Permute from interleaved to planer format, does we need to specify yzcx as input instead
        // of cxyz
        auto img = util::make_unique<cimg_library::CImg<T>>(
            glm::value_ptr(*typedDataPtr), static_cast<unsigned int>(dataFormat->getComponents()),
            static_cast<unsigned int>(inputLayerRAM->getDimensions().x),
            static_cast<unsigned int>(inputLayerRAM->getDimensions().y), 1u, false);

        if (permute) img->permute_axes("yzcx");

        return img;
    }
};

struct CImgNormalizedLayerDispatcher {
    using type = std::unique_ptr<std::vector<unsigned char>>;
    template <typename T>
    std::unique_ptr<std::vector<unsigned char>> dispatch(const LayerRAM* inputLayer) {
        auto img = LayerToCImg<typename T::type>::convert(inputLayer, false);

        // TODO this does not work for signed char... 255 out of range
        cimg_library::CImg<unsigned char> normalizedImg = img->get_normalize(0, 255);
        normalizedImg.mirror('z');

        if (inputLayer->getDataFormat()->getComponents() == 1) {
            normalizedImg.mirror('y');
        }

        auto data = util::make_unique<std::vector<unsigned char>>(
            &normalizedImg[0], &normalizedImg[normalizedImg.size()]);

        return data;
    }
};

struct CImgLoadLayerDispatcher {
    using type = void*;
    template <typename Result, typename DF>
    void* operator()(void* dst, const std::string& filePath, uvec2& dimensions,
                     DataFormatId& formatId, bool rescaleToDim) {
        using P = typename DF::primitive;

        const DataFormatBase* dataFormat = DF::get();

        try {
            cimg_library::CImg<P> img(filePath.c_str());
            size_t components = static_cast<size_t>(img.spectrum());

            if (rescaleToDim) {
                img.resize(dimensions.x, dimensions.y, -100, -100, 3);
            } else {
                dimensions = uvec2(img.width(), img.height());
            }

            auto loadedDataFormat =
                DataFormatBase::get(dataFormat->getNumericType(), components, sizeof(P) * 8);
            if (loadedDataFormat) {
                formatId = loadedDataFormat->getId();
            } else {
                throw DataReaderException(
                    "CImgLoadLayerDispatcher, could not find proper data type", IvwContext);
            }

            // Image is up-side-down
            img.mirror('y');

            return CImgToVoidConvert<P>::convert(dst, &img);
        } catch (cimg_library::CImgIOException& e) {
            throw DataReaderException(std::string(e.what()), IvwContext);
        }
    }
};

struct CImgSaveLayerDispatcher {
    using type = void;
    template <typename Result, typename T>
    void operator()(const std::string& filePath, const LayerRAM* inputLayer) {
        auto img = LayerToCImg<typename T::type>::convert(inputLayer);

        // Should rescale values based on output format i.e. PNG/JPG is 0-255, HDR different.
        const DataFormatBase* outFormat = DataFloat32::get();
        std::string fileExtension = toLower(filesystem::getFileExtension(filePath));
        if (extToBaseTypeMap_.find(fileExtension) != extToBaseTypeMap_.end()) {
            outFormat = DataFormatBase::get(extToBaseTypeMap_[fileExtension]);
        }

        // Image is up-side-down
        img->mirror('y');

        const DataFormatBase* inFormat = inputLayer->getDataFormat();
        double inMin = inFormat->getMin();
        double inMax = inFormat->getMax();
        double outMin = outFormat->getMin();
        double outMax = outFormat->getMax();

        // Special treatment for float data types:
        // For float input images, we assume that the range is [0,1] (which is the same as rendered
        // in a Canvas)
        // For float output images, we normalize to [0,1]
        // Note that no normalization is performed if both input and output are float images
        if (inFormat->getNumericType() == NumericType::Float) {
            inMin = 0.0;
            inMax = 1.0;
        }
        if (outFormat->getNumericType() == NumericType::Float) {
            outMin = 0.0;
            outMax = 1.0;
        }

        // The image values should be rescaled if the ranges of the input and output are different
        if (inMin != outMin || inMax != outMax) {
            typename T::primitive* data = img->data();
            double scale = (outMax - outMin) / (inMax - inMin);
            for (size_t i = 0; i < img->size(); i++) {
                auto dataValue = glm::clamp(static_cast<double>(data[i]), inMin, inMax);
                data[i] = static_cast<typename T::primitive>((dataValue - inMin) * scale + outMin);
            }
        }
        try {
            img->save(filePath.c_str());
        } catch (cimg_library::CImgIOException& e) {
            throw DataWriterException(
                "Failed to save image to: " + filePath + " Reason: " + std::string(e.what()),
                IvwContext);
        }
    }
};

struct CImgSaveLayerToBufferDispatcher {
    using type = std::unique_ptr<std::vector<unsigned char>>;
    template <typename Result, typename T>
    type operator()(const LayerRAM* inputLayer, const std::string& extension) {
        auto img = LayerToCImg<typename T::type>::convert(inputLayer);

        // Should rescale values based on output format i.e. PNG/JPG is 0-255, HDR different.
        const DataFormatBase* outFormat = DataFloat32::get();
        std::string fileExtension = toLower(extension);
        if (extToBaseTypeMap_.find(fileExtension) != extToBaseTypeMap_.end()) {
            outFormat = DataFormatBase::get(extToBaseTypeMap_[fileExtension]);
        }

        // Image is up-side-down
        img->mirror('y');

        const DataFormatBase* inFormat = inputLayer->getDataFormat();
        double inMin = inFormat->getMin();
        double inMax = inFormat->getMax();
        double outMin = outFormat->getMin();
        double outMax = outFormat->getMax();

        // Special treatment for float data types:
        // For float input images, we assume that the range is [0,1] (which is the same as rendered
        // in a Canvas)
        // For float output images, we normalize to [0,1]
        // Note that no normalization is performed if both input and output are float images
        if (inFormat->getNumericType() == NumericType::Float) {
            inMin = 0.0;
            inMax = 1.0;
        }
        if (outFormat->getNumericType() == NumericType::Float) {
            outMin = 0.0;
            outMax = 1.0;
        }

        // The image values should be rescaled if the ranges of the input and output are different
        if (inMin != outMin || inMax != outMax) {
            typename T::primitive* data = img->data();
            double scale = (outMax - outMin) / (inMax - inMin);
            for (size_t i = 0; i < img->size(); i++) {
                auto dataValue = glm::clamp(static_cast<double>(data[i]), inMin, inMax);
                data[i] = static_cast<typename T::primitive>((dataValue - inMin) * scale + outMin);
            }
        }
        try {
            return std::make_unique<std::vector<unsigned char>>(
                std::move(cimgutil::saveCImgToBuffer(*img.get(), extension)));
        } catch (cimg_library::CImgIOException& e) {
            throw DataWriterException(
                "Failed to save image to buffer. Reason: " + std::string(e.what()), IvwContext);
        }
    }
};

struct CImgRescaleLayerDispatcher {
    using type = void*;
    template <typename Result, typename T>
    void* operator()(const LayerRAM* inputLayerRAM, uvec2 dst_dim) {
        auto img = LayerToCImg<typename T::type>::convert(inputLayerRAM);

        img->resize(dst_dim.x, dst_dim.y, -100, -100, 3);

        return CImgToVoidConvert<typename T::primitive>::convert(nullptr, img.get());
    }
};

struct CImgLoadVolumeDispatcher {
    using type = void*;
    template <typename Result, typename DF>
    void* operator()(void* dst, const std::string& filePath, size3_t& dimensions,
                     DataFormatId& formatId) {
        const DataFormatBase* dataFormat = DF::get();

        cimg_library::CImg<typename DF::primitive> img(filePath.c_str());

        size_t components = static_cast<size_t>(img.spectrum());
        dimensions = size3_t(img.width(), img.height(), img.depth());

        const DataFormatBase* loadedDataFormat = DataFormatBase::get(
            dataFormat->getNumericType(), components, sizeof(typename DF::primitive) * 8);
        if (loadedDataFormat) {
            formatId = loadedDataFormat->getId();
        } else {
            throw Exception("CImgLoadVolumeDispatcher, could not find proper data type");
        }

        // Image is up-side-down
        img.mirror('y');

        return CImgToVoidConvert<typename DF::primitive>::convert(dst, &img);
    }
};

////////////////////// CImgUtils ///////////////////////////////////////////////////

void* loadLayerData(void* dst, const std::string& filePath, uvec2& dimensions,
                    DataFormatId& formatId, bool rescaleToDim) {
    std::string fileExtension = toLower(filesystem::getFileExtension(filePath));
    if (extToBaseTypeMap_.find(fileExtension) != extToBaseTypeMap_.end()) {
        formatId = extToBaseTypeMap_[fileExtension];
    } else {
        formatId = DataFormatId::Float32;
    }

    CImgLoadLayerDispatcher disp;
    return dispatching::dispatch<void*, dispatching::filter::All>(
        formatId, disp, dst, filePath, dimensions, formatId, rescaleToDim);
}

void* loadVolumeData(void* dst, const std::string& filePath, size3_t& dimensions,
                     DataFormatId& formatId) {
    std::string fileExtension = toLower(filesystem::getFileExtension(filePath));
    if (extToBaseTypeMap_.find(fileExtension) != extToBaseTypeMap_.end()) {
        formatId = extToBaseTypeMap_[fileExtension];
    } else {
        formatId = DataFormatId::Float32;
    }

    CImgLoadVolumeDispatcher disp;
    return dispatching::dispatch<void*, dispatching::filter::All>(formatId, disp, dst, filePath,
                                                                  dimensions, formatId);
}

void saveLayer(const std::string& filePath, const Layer* inputLayer) {
    CImgSaveLayerDispatcher disp;
    const LayerRAM* inputLayerRam = inputLayer->getRepresentation<LayerRAM>();

    return dispatching::dispatch<void, dispatching::filter::All>(
        inputLayerRam->getDataFormat()->getId(), disp, filePath, inputLayerRam);
}

std::unique_ptr<std::vector<unsigned char>> saveLayerToBuffer(const std::string& extension,
                                                              const Layer* inputLayer) {
    CImgSaveLayerToBufferDispatcher disp;
    const LayerRAM* inputLayerRam = inputLayer->getRepresentation<LayerRAM>();
    return dispatching::dispatch<std::unique_ptr<std::vector<unsigned char>>,
                                 dispatching::filter::All>(inputLayerRam->getDataFormat()->getId(),
                                                           disp, inputLayerRam, extension);
}

void* rescaleLayer(const Layer* inputLayer, uvec2 dst_dim) {
    const LayerRAM* layerRam = inputLayer->getRepresentation<LayerRAM>();
    return rescaleLayerRAM(layerRam, dst_dim);
}

void* rescaleLayerRAM(const LayerRAM* srcLayerRam, uvec2 dst_dim) {
    CImgRescaleLayerDispatcher disp;
    return dispatching::dispatch<void*, dispatching::filter::All>(
        srcLayerRam->getDataFormat()->getId(), disp, srcLayerRam, dst_dim);
}

struct CImgRescaleLayerRamToLayerRamDispatcher {
    using type = bool;
    template <typename Result, typename T>
    bool operator()(const LayerRAM* source, LayerRAM* target) {
        using E = typename T::type;       // elem type i.e. vec3
        using P = typename T::primitive;  // comp type i.e float
        const size_t rank = util::rank<E>::value;

        const uvec2 sourceDim = source->getDimensions();
        const uvec2 targetDim = target->getDimensions();

        const double sourceAspect =
            static_cast<double>(sourceDim.x) / static_cast<double>(sourceDim.y);
        const double targetAspect =
            static_cast<double>(targetDim.x) / static_cast<double>(targetDim.y);

        const uvec2 resizeDim{
            sourceAspect > targetAspect ? targetDim.x : targetDim.y * sourceAspect,
            sourceAspect > targetAspect ? targetDim.x / sourceAspect : targetDim.y};

        auto srcData = static_cast<const P*>(source->getData());
        P* dstData = static_cast<P*>(target->getData());

        if (rank == 0) {
            cimg_library::CImg<P> src(srcData, sourceDim.x, sourceDim.y, 1, 1, true);
            auto resized = src.get_resize(resizeDim.x, resizeDim.y, -100, -100,
                                          static_cast<int>(InterpolationType::Linear));

            cimg_library::CImg<P> dst(dstData, targetDim.x, targetDim.y, 1, 1, true);
            dst.fill(P{0});
            dst.draw_image(targetDim.x / 2 - resizeDim.x / 2, targetDim.y / 2 - resizeDim.y / 2,
                           resized);
        } else {
            // Inviwo store pixels interleaved (RGBRGBRGB),
            // CImg stores pixels in a planer format (RRRRGGGGBBBB).
            // Permute from interleaved to planer format,
            // we need to specify yzcx as input instead of cxyz

            size_t comp = util::extent<E>::value;

            cimg_library::CImg<P> src(srcData, comp, sourceDim.x, sourceDim.y, 1, true);
            auto temp = src.get_permute_axes("yzcx");  // put first index last

            temp.resize(resizeDim.x, resizeDim.y, -100, -100,
                        static_cast<int>(InterpolationType::Linear));

            cimg_library::CImg<P> dst(dstData, targetDim.x, targetDim.y, 1, comp, true);
            dst.fill(P{0});
            dst.draw_image(targetDim.x / 2 - resizeDim.x / 2, targetDim.y / 2 - resizeDim.y / 2,
                           temp);

            dst.permute_axes("cxyz");  // put last index first
        }

        return true;
    }
};

bool rescaleLayerRamToLayerRam(const LayerRAM* source, LayerRAM* target) {
    if (!source->getData()) return false;
    if (!target->getData()) return false;
    if (source->getDataFormatId() != target->getDataFormatId()) return false;

    CImgRescaleLayerRamToLayerRamDispatcher disp;
    return dispatching::dispatch<bool, dispatching::filter::All>(source->getDataFormat()->getId(),
                                                                 disp, source, target);
}

std::string getLibJPGVesrion() {
#ifdef cimg_use_jpeg
    std::ostringstream oss;
    oss << JPEG_LIB_VERSION_MAJOR << "." << JPEG_LIB_VERSION_MINOR;
    return oss.str();
#else
    return "LibJPG not available";
#endif
}

std::string getOpenEXRVesrion() {
#ifdef cimg_use_openexr
    std::ostringstream oss;
    oss << OPENEXR_VERSION_MAJOR << "." << OPENEXR_VERSION_MINOR << "." << OPENEXR_VERSION_PATCH;
    return oss.str();
#else
    return "OpenEXR not available";
#endif
}

}  // namespace cimgutil

}  // namespace inviwo

#include <warn/pop>
