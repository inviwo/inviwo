/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/opencl/image/layercl.h>
#include <modules/opencl/image/layerclresizer.h>
#include <modules/opencl/clformats.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/glmutils.h>

namespace inviwo {

LayerCL::LayerCL(size2_t dimensions, LayerType type, const DataFormatBase* format,
                 const SwizzleMask& swizzleMask, InterpolationType interpolation,
                 const Wrapping2D& wrapping, const void* data)
    : LayerCLBase()
    , LayerRepresentation(type)
    , dataFormatBase_{format}
    , dimensions_(dimensions)
    , layerFormat_(dataFormatToCLImageFormat(format->getId()))
    , swizzleMask_(swizzleMask)
    , interpolation_{interpolation}
    , wrapping_{wrapping} {
    initialize(data);
}
LayerCL::LayerCL(const LayerReprConfig& config, const void* data)
    : LayerCL{config.dimensions.value_or(LayerConfig::defaultDimensions),
              config.type.value_or(LayerConfig::defaultType),
              config.format ? config.format : LayerConfig::defaultFormat,
              config.swizzleMask.value_or(LayerConfig::defaultSwizzleMask),
              config.interpolation.value_or(LayerConfig::defaultInterpolation),
              config.wrapping.value_or(LayerConfig::defaultWrapping),
              data} {}

LayerCL::LayerCL(const LayerCL& rhs)
    : LayerCLBase(rhs)
    , LayerRepresentation(rhs)
    , dataFormatBase_{rhs.dataFormatBase_}
    , dimensions_(rhs.dimensions_)
    , layerFormat_(rhs.layerFormat_)
    , swizzleMask_(rhs.swizzleMask_)
    , interpolation_{rhs.interpolation_}
    , wrapping_{rhs.wrapping_} {
    initialize(nullptr);
    OpenCL::getPtr()->getQueue().enqueueCopyImage(rhs.get(), *clImage_, glm::size3_t(0),
                                                  glm::size3_t(0), glm::size3_t(dimensions_, 1));
}

LayerCL::~LayerCL() = default;

void LayerCL::initialize(const void* texels) {
    if (texels != nullptr) {
        // Could performance be increased by using pinned memory?
        // 3.1.1
        // http://www.nvidia.com/content/cudazone/CUDABrowser/downloads/papers/NVIDIA_OpenCL_BestPracticesGuide.pdf
        // cl::Buffer pinnedMem(OpenCL::getPtr()->getContext(), CL_MEM_READ_ONLY |
        // CL_MEM_ALLOC_HOST_PTR, sizeof(texels), nullptr, nullptr);
        // unsigned char* mappedMem = (unsigned
        // char*)OpenCL::getPtr()->getQueue().enqueueMapBuffer(pinnedMem, true, CL_MAP_WRITE, 0,
        // sizeof(texels), 0);
        // memcpy(mappedMem, texels, sizeof(texels));
        // OpenCL::getPtr()->getQueue().enqueueWriteLayer(*layer2D_, true, glm::size3_t(0),
        // glm::size3_t(dimensions_, 1), 0, 0, mappedMem);
        // OpenCL::getPtr()->getQueue().enqueueUnmapMemObject(pinnedMem, mappedMem);
        // This should also use pinned memory...
        clImage_ = std::make_unique<cl::Image2D>(
            OpenCL::getPtr()->getContext(),
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR | CL_MEM_ALLOC_HOST_PTR, getFormat(),
            static_cast<size_t>(dimensions_.x), static_cast<size_t>(dimensions_.y), 0,
            const_cast<void*>(texels));
        // Alternatively first allocate memory on device and then transfer
        // layer2D_ = new cl::Layer2D(OpenCL::getPtr()->getContext(), CL_MEM_READ_WRITE,
        // getFormat(), dimensions_.x, dimensions_.y);
        // OpenCL::getPtr()->getQueue().enqueueWriteLayer(*layer2D_, true, glm::size3_t(0),
        // glm::size3_t(dimensions_, 1), 0, 0, texels);
    } else {
        clImage_ = std::make_unique<cl::Image2D>(OpenCL::getPtr()->getContext(), CL_MEM_READ_WRITE,
                                                 getFormat(), static_cast<size_t>(dimensions_.x),
                                                 static_cast<size_t>(dimensions_.y));
    }
}

LayerCL* LayerCL::clone() const { return new LayerCL(*this); }

void LayerCL::upload(const void* data) {
    OpenCL::getPtr()->getQueue().enqueueWriteImage(*clImage_, true, glm::size3_t(0),
                                                   glm::size3_t(dimensions_, 1), 0, 0,
                                                   const_cast<void*>(data));
}

void LayerCL::download(void* data) const {
    OpenCL::getPtr()->getQueue().enqueueReadImage(*clImage_, true, glm::size3_t(0),
                                                  glm::size3_t(dimensions_, 1), 0, 0, data);
}

bool LayerCL::copyRepresentationsTo(LayerRepresentation* target) const {
    LayerCL* targetCL = dynamic_cast<LayerCL*>(target);

    if (!targetCL) return false;

    LayerCLResizer::resize(*clImage_, (targetCL->get()), targetCL->getDimensions());
    return true;
}

std::type_index LayerCL::getTypeIndex() const { return std::type_index(typeid(LayerCL)); }

dvec4 LayerCL::readPixel(size2_t pos) const {
    return dispatching::singleDispatch<dvec4, dispatching::filter::All>(
        getDataFormat()->getId(), [&]<typename T>() {
            T res{};

            if constexpr (util::rank_v<T> == 0) {
                OpenCL::getPtr()->getQueue().enqueueReadImage(*clImage_, true, glm::size3_t(pos, 0),
                                                              glm::size3_t(1, 1, 1), 0, 0,
                                                              static_cast<void*>(&res));
            } else {
                OpenCL::getPtr()->getQueue().enqueueReadImage(
                    *clImage_, true, glm::size3_t(pos, 0), glm::size3_t(1, 1, 1), 0, 0,
                    static_cast<void*>(glm::value_ptr(res)));
            }
            return util::glm_convert<dvec4>(res);
        });
}

void LayerCL::setDimensions(size2_t dimensions) {
    if (dimensions == dimensions_) {
        return;
    }

    try {
        auto resizedLayer2D =
            std::make_unique<cl::Image2D>(OpenCL::getPtr()->getContext(), CL_MEM_READ_WRITE,
                                          getFormat(), dimensions.x, dimensions.y);
        LayerCLResizer::resize(*clImage_, *resizedLayer2D, dimensions);
        clImage_ = std::move(resizedLayer2D);
    } catch (const cl::Error& err) {
        if (err.err() == CL_INVALID_IMAGE_DESCRIPTOR) {
            // OpenCL images with (0,0) sizes throws exception
            // Most likely during image port disconnection, so we probably want to ignore it.
            LogError(getCLErrorString(err));
        } else {
            throw err;
        }
    }

    dimensions_ = dimensions;
}

const size2_t& LayerCL::getDimensions() const { return dimensions_; }

void LayerCL::setSwizzleMask(const SwizzleMask& mask) { swizzleMask_ = mask; }

SwizzleMask LayerCL::getSwizzleMask() const { return swizzleMask_; }

void LayerCL::setInterpolation(InterpolationType interpolation) { interpolation_ = interpolation; }

InterpolationType LayerCL::getInterpolation() const { return interpolation_; }

void LayerCL::setWrapping(const Wrapping2D& wrapping) { wrapping_ = wrapping; }

Wrapping2D LayerCL::getWrapping() const { return wrapping_; }

}  // namespace inviwo

namespace cl {

template <>
cl_int Kernel::setArg(cl_uint index, const inviwo::LayerCL& value) {
    return setArg(index, value.get());
}

}  // namespace cl
