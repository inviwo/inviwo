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

#if not defined(__clang__) and defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif

#include <modules/cimg/cimgutils.h>

#include <inviwo/core/datastructures/image/imagetypes.h>                // for SwizzleMask, lumi...
#include <inviwo/core/datastructures/image/layerram.h>                  // for LayerRAM
#include <inviwo/core/datastructures/volume/volumeram.h>                // for LayerRAM
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/io/datareaderexception.h>                         // for DataReaderException
#include <inviwo/core/io/datawriterexception.h>                         // for DataWriterException
#include <inviwo/core/util/exception.h>                                 // for Exception
#include <inviwo/core/util/filesystem.h>                                // for getFileExtension
#include <inviwo/core/util/formatdispatching.h>                         // for dispatch, All
#include <inviwo/core/util/formats.h>                                   // for DataFormatId, Dat...
#include <inviwo/core/util/glmutils.h>                                  // for extent, rank
#include <inviwo/core/util/glmvec.h>                                    // for uvec2, size3_t
#include <inviwo/core/util/raiiutils.h>                                 // for OnScopeExit, OnSc...
#include <inviwo/core/util/safecstr.h>                                  // for SafeCStr
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT, IVW_...
#include <inviwo/core/util/stringconversion.h>                          // for toLower
#include <modules/cimg/cimgsavebuffer.h>                                // for saveCImgToBuffer

#include <algorithm>      // for min
#include <cstdint>        // for uint16_t, uint32_t
#include <cstring>        // for size_t, memcpy
#include <functional>     // for __base
#include <ostream>        // for operator<<, basic...
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <utility>        // for move

#include <fmt/std.h>
#if not defined(__clang__) and defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include <warn/push>
#include <warn/ignore/all>
#if defined(__clang__)
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wformat-truncation"
#elif (_MSC_VER)
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
#include <CImg.h>                    // for CImgException
#include <fmt/core.h>                // for format
#include <glm/common.hpp>            // for clamp
#include <glm/detail/qualifier.hpp>  // for qualifier
#include <glm/detail/setup.hpp>      // for length_t
#include <glm/gtc/type_ptr.hpp>      // for value_ptr
#include <glm/vec2.hpp>              // for vec<>::(anonymous)
#include <jpeglib.h>                 // for JPEG_LIB_VERSION_...
#include <warn/pop>

#include <warn/push>
#include <warn/ignore/switch-enum>
#include <warn/ignore/conversion>
#if defined(__clang__)
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wformat-truncation"
#elif (_MSC_VER)
#pragma warning(disable : 4297)
#pragma warning(disable : 4267)
#endif

#ifdef cimg_use_tiff
#include <tiff.h>    // for SAMPLEFORMAT_COMP...
#include <tiffio.h>  // for TIFFGetFieldDefau...
#endif

#ifdef cimg_use_openexr
#include <OpenEXRConfig.h>  // for OPENEXR_VERSION_M...
#endif

namespace inviwo {

namespace cimgutil {

std::unordered_map<std::string_view, DataFormatId> extToBaseTypeMap_ = {
    {".jpg", DataFormatId::UInt8},
    {".jpeg", DataFormatId::UInt8},
    {".bmp", DataFormatId::UInt8},
    {".exr", DataFormatId::Float32},
    {".hdr", DataFormatId::Float32}};
namespace detail {

cimgutil::InterpolationType toCImgInterpolationType(inviwo::InterpolationType type) {
    switch (type) {
        case inviwo::InterpolationType::Linear:
            return cimgutil::InterpolationType::Linear;
        case inviwo::InterpolationType::Nearest:
            return cimgutil::InterpolationType::Nearest;
        default:
            return cimgutil::InterpolationType::Linear;
    }
}

}  // namespace detail

template <typename T>
auto layerToCImg(const LayerRAMPrecision<T>* src, bool permute = true, bool skipAlpha = false) {

    if constexpr (util::rank_v<T> == 0) {
        return cimg_library::CImg<T>(
            src->getDataTyped(), static_cast<unsigned int>(src->getDimensions().x),
            static_cast<unsigned int>(src->getDimensions().y), 1, 1, false);

    } else if constexpr (util::rank_v<T> == 1) {
        auto* typedDataPtr = src->getDataTyped();

        // Inviwo store pixels interleaved (RGBRGBRGB), CImg stores pixels in a planar format
        // (RRRRGGGGBBBB).
        // Permute from interleaved to planar format, i.e specify yzcx as input instead
        // of cxyz
        auto img = cimg_library::CImg<util::value_type_t<T>>(
            glm::value_ptr(*typedDataPtr), static_cast<unsigned int>(util::extent_v<T>),
            static_cast<unsigned int>(src->getDimensions().x),
            static_cast<unsigned int>(src->getDimensions().y), 1u, false);

        if (permute) img.permute_axes("yzcx");
        if (skipAlpha && img.spectrum() > 1) {
            img.channels(0, img.spectrum() - 2);
        }
        return img;
    } else {
        static_assert(util::alwaysFalse<T>(), "unsupported type");
    }
}

namespace {
DataFormatId findScalarFormat(const std::filesystem::path& filePath) {
    const auto fileExtension = toLower(filePath.extension().string());
    if (extToBaseTypeMap_.find(fileExtension) != extToBaseTypeMap_.end()) {
        return extToBaseTypeMap_[fileExtension];
    } else {
        return DataFormatId::Float32;
    }
}

}  // namespace

std::shared_ptr<LayerRAM> loadLayer(const std::filesystem::path& filePath) {
    const auto scalarFormatId = findScalarFormat(filePath);

    return dispatching::singleDispatch<std::shared_ptr<LayerRAM>, dispatching::filter::Scalars>(
        scalarFormatId, [&]<typename Scalar>() -> std::shared_ptr<LayerRAM> {
            cimg_library::CImg<Scalar> img(filePath.string().c_str());
            const auto components = static_cast<size_t>(img.spectrum());
            const auto dimensions = size2_t{img.width(), img.height()};
            img.mirror('y');  // Image is up-side-down
            if (components > 1) {
                // Inviwo store pixels interleaved (RGBRGBRGB), CImg stores pixels in a planar
                // format (RRRRGGGGBBBB). Permute from planar to interleaved format, hence we need
                // to specify cxyz as input instead of xyzc
                img.permute_axes("cxyz");
            }

            const auto scalarFormat = DataFormatBase::get(scalarFormatId);
            const auto imageFormat = DataFormatBase::get(scalarFormat->getNumericType(), components,
                                                         scalarFormat->getPrecision());
            if (!imageFormat) {
                throw DataReaderException(IVW_CONTEXT_CUSTOM("cimgutil::loadLayer"),
                                          "could not find proper data type");
            }

            return dispatching::singleDispatch<std::shared_ptr<LayerRAM>, dispatching::filter::All>(
                imageFormat->getId(), [&]<typename T>() -> std::shared_ptr<LayerRAM> {
                    constexpr auto swizzleMask = swizzlemasks::defaultColor(util::extent_v<T>);
                    img._is_shared = true;  // Steal the buffer from img
                    return std::make_shared<LayerRAMPrecision<T>>(
                        static_cast<T*>(static_cast<void*>(img._data)), dimensions,
                        LayerType::Color, swizzleMask);
                });
        });
}

std::shared_ptr<LayerRAM> loadLayerTiff(const std::filesystem::path& filePath) {
    const auto header = cimgutil::getTIFFHeader(filePath);

    return dispatching::singleDispatch<std::shared_ptr<LayerRAM>, dispatching::filter::All>(
        header.format->getId(), [&]<typename T>() -> std::shared_ptr<LayerRAM> {
            using Scalar = util::value_type_t<T>;

            cimg_library::CImg<Scalar> img(filePath.string().c_str());
            const auto components = static_cast<size_t>(img.spectrum());
            const auto dimensions = size2_t{img.width(), img.height()};
            img.mirror('y');  // Image is up-side-down
            if (components > 1) {
                // Inviwo store pixels interleaved (RGBRGBRGB), CImg stores pixels in a planar
                // format (RRRRGGGGBBBB). Permute from planar to interleaved format, hence we need
                // to specify cxyz as input instead of xyzc
                img.permute_axes("cxyz");
            }

            if (size2_t{header.dimensions} != dimensions) {
                throw DataReaderException(IVW_CONTEXT_CUSTOM("cimgutil::loadLayerTiff"),
                                          "Image size missmatch");
            }
            if (util::extent_v<T> != components) {
                throw DataReaderException(IVW_CONTEXT_CUSTOM("cimgutil::loadLayerTiff"),
                                          "Image component missmatch");
            }

            img._is_shared = true;  // Steal the buffer from img
            return std::make_shared<LayerRAMPrecision<T>>(
                static_cast<T*>(static_cast<void*>(img._data)), size2_t{header.dimensions},
                LayerType::Color, header.swizzleMask);
        });
}

std::shared_ptr<VolumeRAM> loadVolume(const std::filesystem::path& filePath) {
    const auto scalarFormatId = findScalarFormat(filePath);

    return dispatching::singleDispatch<std::shared_ptr<VolumeRAM>, dispatching::filter::Scalars>(
        scalarFormatId, [&]<typename Scalar>() -> std::shared_ptr<VolumeRAM> {
            cimg_library::CImg<Scalar> img(filePath.string().c_str());
            const auto components = static_cast<size_t>(img.spectrum());
            const auto dimensions = size3_t{img.width(), img.height(), img.depth()};

            img.mirror("y");  // Image is up-side-down

            const auto scalarFormat = DataFormatBase::get(scalarFormatId);
            const auto imageFormat = DataFormatBase::get(scalarFormat->getNumericType(), components,
                                                         scalarFormat->getPrecision());
            if (!imageFormat) {
                throw DataReaderException(IVW_CONTEXT_CUSTOM("cimgutil::loadVolume"),
                                          "could not find proper data type");
            }

            return dispatching::singleDispatch<std::shared_ptr<VolumeRAM>,
                                               dispatching::filter::All>(
                imageFormat->getId(), [&]<typename T>() -> std::shared_ptr<VolumeRAM> {
                    img._is_shared = true;  // Steal the buffer from img
                    return std::make_shared<VolumeRAMPrecision<T>>(
                        static_cast<T*>(static_cast<void*>(img._data)), dimensions);
                });
        });
}

std::shared_ptr<VolumeRAM> loadVolume(const std::filesystem::path& filePath,
                                      const DataFormatBase* format, size3_t dims) {
    return dispatching::singleDispatch<std::shared_ptr<VolumeRAM>, dispatching::filter::All>(
        format->getId(), [&]<typename T>() -> std::shared_ptr<VolumeRAM> {
            using Scalar = util::value_type_t<T>;
            cimg_library::CImg<Scalar> img(filePath.string().c_str());
            const auto components = static_cast<size_t>(img.spectrum());
            const auto dimensions = size3_t{img.width(), img.height(), img.depth()};
            img.mirror("y");  // Image is up-side-down

            if (dims != dimensions) {
                throw DataReaderException(IVW_CONTEXT_CUSTOM("cimgutil::updateVolume"),
                                          "Volume size missmatch");
            }
            if (util::extent_v<T> != components) {
                throw DataReaderException(IVW_CONTEXT_CUSTOM("cimgutil::updateVolume"),
                                          "Volume component missmatch");
            }

            img._is_shared = true;  // Steal the buffer from img
            return std::make_shared<VolumeRAMPrecision<T>>(
                static_cast<T*>(static_cast<void*>(img._data)), dimensions);
        });
}

void updateVolume(VolumeRAM& volume, const std::filesystem::path& filePath) {
    volume.dispatch<void, dispatching::filter::All>([&]<typename T>(VolumeRAMPrecision<T>* vr) {
        using Scalar = util::value_type_t<T>;
        cimg_library::CImg<Scalar> img(filePath.string().c_str());
        const auto components = static_cast<size_t>(img.spectrum());
        const auto dimensions = size3_t{img.width(), img.height(), img.depth()};
        img.mirror("y");  // Image is up-side-down

        if (vr->getDimensions() != dimensions) {
            throw DataReaderException(IVW_CONTEXT_CUSTOM("cimgutil::updateVolume"),
                                      "Volume size missmatch");
        }
        if (util::extent_v<T> != components) {
            throw DataReaderException(IVW_CONTEXT_CUSTOM("cimgutil::updateVolume"),
                                      "Volume component missmatch");
        }
        auto* dst = static_cast<Scalar*>(vr->getData());
        std::copy(img.data(), img.data() + glm::compMul(dimensions) * components, dst);
    });
}

template <typename T>
auto prepareImgFromLayer(const LayerRAMPrecision<T>* lr, std::string_view fileExtension) {
    const bool isJpeg = (fileExtension == ".jpg") || (fileExtension == ".jpeg");
    const bool skipAlpha = isJpeg && ((util::extent_v<T> == 2) || (util::extent_v<T> == 4));
    auto img = layerToCImg(lr, true, skipAlpha);

    const DataFormatBase* inFormat = DataFormat<T>::get();
    // Should rescale values based on output format i.e. PNG/JPG is 0-255, HDR different.
    const DataFormatBase* outFormat = DataFloat32::get();
    if (extToBaseTypeMap_.find(fileExtension) != extToBaseTypeMap_.end()) {
        outFormat = DataFormatBase::get(extToBaseTypeMap_[fileExtension]);
    } else if ((fileExtension == ".tif") || (fileExtension == ".tiff")) {
        // use the same data format as the input. TIFF supports 8 and 16 bit integer formats as
        // well as 32 bit floating point
        const size_t maxPrecision = (inFormat->getNumericType() == NumericType::Float) ? 32 : 16;
        const size_t bitsPerSample = std::min<size_t>(inFormat->getPrecision(), maxPrecision);
        outFormat =
            DataFormatBase::get(inFormat->getNumericType(), util::extent_v<T>, bitsPerSample);
    }

    // Image is up-side-down
    img.mirror('y');

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
        util::value_type_t<T>* data = img.data();
        const double scale = (outMax - outMin) / (inMax - inMin);
        for (size_t i = 0; i < img.size(); i++) {
            auto dataValue = glm::clamp(static_cast<double>(data[i]), inMin, inMax);
            data[i] = static_cast<util::value_type_t<T>>((dataValue - inMin) * scale + outMin);
        }
    }
    return img;
}

void saveLayer(const LayerRAM& layer, const std::filesystem::path& filePath) {
    layer.dispatch<void>([&]<typename T>(const LayerRAMPrecision<T>* lr) {
        const auto fileExtension = toLower(filePath.extension().string());
        auto img = prepareImgFromLayer(lr, fileExtension);
        try {
            img.save(filePath.string().c_str());
        } catch (cimg_library::CImgIOException& e) {
            throw DataWriterException(IVW_CONTEXT_CUSTOM("cimgutil::saveLayer"),
                                      "Failed to save image to: {} Reason: {}", filePath, e.what());
        }
    });
}

void saveLayer(const LayerRAM& layer, std::vector<unsigned char>& dst, std::string_view extension) {
    layer.dispatch<void>([&]<typename T>(const LayerRAMPrecision<T>* lr) {
        auto img = prepareImgFromLayer(lr, extension);
        try {
            cimgutil::saveCImgToBuffer(img, extension, dst);
        } catch (cimg_library::CImgIOException& e) {
            throw DataWriterException(
                IVW_CONTEXT_CUSTOM("cimgutil::saveLayer"),
                "Failed to save image to buffer. Reason: " + std::string(e.what()));
        }
    });
}

bool rescaleLayerRamToLayerRam(const LayerRAM* source, LayerRAM* target) {
    if (!source->getData()) return false;
    if (!target->getData()) return false;
    if (source->getDataFormatId() != target->getDataFormatId()) return false;

    return source->dispatch<bool, dispatching::filter::All>(
        [&]<typename T>(const LayerRAMPrecision<T>* srcRep) {
            using P = util::value_type_t<T>;  // comp type i.e float
            const size_t rank = util::rank<T>::value;

            const uvec2 sourceDim = srcRep->getDimensions();
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

            auto interpolation = detail::toCImgInterpolationType(source->getInterpolation());

            if (rank == 0) {
                cimg_library::CImg<P> src(srcData, sourceDim.x, sourceDim.y, 1, 1, true);
                auto resized = src.get_resize(resizeDim.x, resizeDim.y, -100, -100,
                                              static_cast<int>(interpolation));

                cimg_library::CImg<P> dst(dstData, targetDim.x, targetDim.y, 1, 1, true);
                dst.fill(P{0});
                dst.draw_image(targetDim.x / 2 - resizeDim.x / 2, targetDim.y / 2 - resizeDim.y / 2,
                               resized);
            } else {
                // Inviwo store pixels interleaved (RGBRGBRGB),
                // CImg stores pixels in a planar format (RRRRGGGGBBBB).
                // Permute from interleaved to planar format,
                // we need to specify yzcx as input instead of cxyz

                size_t comp = util::extent<T>::value;

                cimg_library::CImg<P> src(srcData, comp, sourceDim.x, sourceDim.y, 1, true);
                auto temp = src.get_permute_axes("yzcx");  // put first index last

                temp.resize(resizeDim.x, resizeDim.y, -100, -100, static_cast<int>(interpolation));

                cimg_library::CImg<P> dst(dstData, targetDim.x, targetDim.y, 1, comp, true);
                dst.fill(P{0});
                dst.draw_image(targetDim.x / 2 - resizeDim.x / 2, targetDim.y / 2 - resizeDim.y / 2,
                               temp);

                dst.permute_axes("cxyz");  // put last index first
            }

            return true;
        });
}

std::string getLibJPGVersion() {
#ifdef cimg_use_jpeg
    std::ostringstream oss;
#if defined(JPEG_LIB_VERSION_MAJOR) && defined(JPEG_LIB_VERSION_MINOR)
    oss << JPEG_LIB_VERSION_MAJOR << "." << JPEG_LIB_VERSION_MINOR;
#else
    oss << JPEG_LIB_VERSION;
#endif
    return oss.str();
#else
    return "LibJPG not available";
#endif
}

std::string getOpenEXRVersion() {
#ifdef cimg_use_openexr
    std::ostringstream oss;
    oss << OPENEXR_VERSION_MAJOR << "." << OPENEXR_VERSION_MINOR << "." << OPENEXR_VERSION_PATCH;
    return oss.str();
#else
    return "OpenEXR not available";
#endif
}

TIFFHeader getTIFFHeader(const std::filesystem::path& filename) {
#ifdef cimg_use_tiff
    TIFF* tif = TIFFOpen(filename.string().c_str(), "r");
    util::OnScopeExit closeFile([tif]() {
        if (tif) TIFFClose(tif);
    });

    if (!tif) {
        throw DataReaderException(IVW_CONTEXT_CUSTOM("cimgutil::getTIFFDataFormat()"),
                                  "Error could not open input file: {}", filename);
    }
    TIFFSetDirectory(tif, 0);

    std::uint16_t samplesPerPixel = 1, bitsPerSample = 8, sampleFormat = 1;
    if (!TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample)) {
        bitsPerSample = 8;
    }
    if (!TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel)) {
        samplesPerPixel = 1;
    }
    if (!TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLEFORMAT, &sampleFormat)) {
        sampleFormat = 1;
    }

    float xres = 1.0f;
    float yres = 1.0f;
    // X and Y resolution tags are stored as float
    if (!TIFFGetFieldDefaulted(tif, TIFFTAG_XRESOLUTION, &xres)) {
        xres = 1.0f;
    }
    if (!TIFFGetFieldDefaulted(tif, TIFFTAG_YRESOLUTION, &yres)) {
        yres = 1.0f;
    }
    const dvec2 res{xres, yres};

    std::uint16_t resUnit = 2;
    if (!TIFFGetFieldDefaulted(tif, TIFFTAG_RESOLUTIONUNIT, &resUnit)) {
        resUnit = 2;
    }
    const TIFFResolutionUnit resolutionUnit = static_cast<TIFFResolutionUnit>(resUnit);

    std::uint32_t x = 0, y = 0, z = 0;
    TIFFGetFieldDefaulted(tif, TIFFTAG_IMAGEWIDTH, &x);
    TIFFGetFieldDefaulted(tif, TIFFTAG_IMAGELENGTH, &y);
    // count the images
    do {
        ++z;
    } while (TIFFReadDirectory(tif));

    NumericType numericType;
    switch (sampleFormat) {
        case SAMPLEFORMAT_UINT:
            numericType = NumericType::UnsignedInteger;
            break;
        case SAMPLEFORMAT_INT:
            numericType = NumericType::SignedInteger;
            break;
        case SAMPLEFORMAT_IEEEFP:
            numericType = NumericType::Float;
            break;
        case SAMPLEFORMAT_COMPLEXIEEEFP:
            [[fallthrough]];
        case SAMPLEFORMAT_COMPLEXINT:
            throw DataReaderException("Unsupported TIFF format",
                                      IVW_CONTEXT_CUSTOM("cimgutil::getTIFFDataFormat()"));
            break;
        default:
            numericType = NumericType::UnsignedInteger;
            break;
    }

    SwizzleMask swizzleMask;
    if (samplesPerPixel == 1) {
        swizzleMask = swizzlemasks::luminance;
    } else if (samplesPerPixel == 2) {
        swizzleMask = swizzlemasks::luminanceAlpha;
    } else if (samplesPerPixel == 3) {
        swizzleMask = swizzlemasks::rgb;
    } else if (samplesPerPixel == 4) {
        swizzleMask = swizzlemasks::rgba;
    } else {
        throw DataReaderException("Unsupported TIFF format with more than 4 channels",
                                  IVW_CONTEXT_CUSTOM("cimgutil::getTIFFDataFormat()"));
    }

    auto df = DataFormatBase::get(numericType, samplesPerPixel, bitsPerSample);

    return {df, size3_t{x, y, z}, res, resolutionUnit, swizzleMask};
#else
    throw Exception("TIFF not available", IVW_CONTEXT_CUSTOM("cimgutil::getTIFFDataFormat()"));
    return {};
#endif
}

}  // namespace cimgutil

}  // namespace inviwo

#include <warn/pop>
