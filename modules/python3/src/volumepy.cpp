/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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

#include <modules/python3/volumepy.h>

#include <inviwo/core/datastructures/image/imagetypes.h>             // for SwizzleMask, Wrapping3D
#include <inviwo/core/datastructures/volume/volumeram.h>             // for createVolumeRAM
#include <inviwo/core/datastructures/volume/volumerepresentation.h>  // for VolumeRepresentation
#include <inviwo/core/util/exception.h>                              // for Exception
#include <inviwo/core/util/formatdispatching.h>                      // for PrecisionValueType
#include <inviwo/core/util/formats.h>                                // for DataFormatBase
#include <inviwo/core/util/glmutils.h>                               // for extent, value_type
#include <inviwo/core/util/glmvec.h>                                 // for size3_t
#include <inviwo/core/util/sourcecontext.h>                          // for IVW_CONTEXT, IVW_CON...
#include <modules/python3/pybindutils.h>                             // for toNumPyFormat, getDa...

#include <inviwo/core/resourcemanager/resource.h>

#include <cstring>      // for memcpy, size_t
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

#include <glm/vec3.hpp>  // for vec<>::(anonymous)

namespace inviwo {

namespace resource {

inline PY toPY(const pybind11::array& data) {
    return resource::PY{pybind11::module_::import("builtins").attr("id")(data).cast<size_t>()};
}

}  // namespace resource

namespace {
const DataFormatBase* format(pybind11::array data) {
    const auto ndim = data.ndim();
    if (!(ndim == 3 || ndim == 4)) {
        throw Exception("Ndims must be either 3 or 4, with 1 <= size[3] <= 4",
                        IVW_CONTEXT_CUSTOM("VolumePy"));
    }
    if ((data.flags() & pybind11::array::c_style) == 0) {
        throw Exception("Numpy array must be C-contiguous.", IVW_CONTEXT_CUSTOM("VolumePy"));
    }

    return pyutil::getDataFormat(ndim == 3 ? 1 : data.shape(3), data);
}
}  // namespace

VolumePy::VolumePy(pybind11::array data, const SwizzleMask& swizzleMask,
                   InterpolationType interpolation, const Wrapping3D& wrapping)
    : VolumeRepresentation()
    , swizzleMask_{swizzleMask}
    , interpolation_{interpolation}
    , wrapping_{wrapping}
    , data_{data}
    , dims_{data_.shape(2), data_.shape(1), data_.shape(0)} {

    resource::add(resource::toPY(data_), Resource{.dims = glm::size4_t{dims_, 0},
                                                  .format = format(data_)->getId(),
                                                  .desc = "VolumePY"});
}

VolumePy::VolumePy(size3_t dimensions, const DataFormatBase* format, const SwizzleMask& swizzleMask,
                   InterpolationType interpolation, const Wrapping3D& wrapping)
    : VolumeRepresentation()
    , swizzleMask_{swizzleMask}
    , interpolation_{interpolation}
    , wrapping_{wrapping}
    , data_{pybind11::array(pyutil::toNumPyFormat(format),
                            pybind11::array::ShapeContainer{dimensions.z, dimensions.y,
                                                            dimensions.x, format->getComponents()})}
    , dims_{dimensions} {

    resource::add(
        resource::toPY(data_),
        Resource{.dims = glm::size4_t{dims_, 0}, .format = format->getId(), .desc = "VolumePY"});
}

VolumePy::VolumePy(const VolumeReprConfig& config)
    : VolumePy{config.dimensions.value_or(VolumeConfig::defaultDimensions),
               config.format ? config.format : VolumeConfig::defaultFormat,
               config.swizzleMask.value_or(VolumeConfig::defaultSwizzleMask),
               config.interpolation.value_or(VolumeConfig::defaultInterpolation),
               config.wrapping.value_or(VolumeConfig::defaultWrapping)} {}

VolumePy::~VolumePy() { resource::remove(resource::toPY(data_)); }

VolumePy* VolumePy::clone() const { return new VolumePy(*this); }

std::type_index VolumePy::getTypeIndex() const { return std::type_index(typeid(VolumePy)); }

const DataFormatBase* VolumePy::getDataFormat() const { return format(data_); }

void VolumePy::setDimensions(size3_t dimensions) {
    if (dimensions != dims_) {
        const auto old = resource::remove(resource::toPY(data_));
        data_ = pybind11::array(
            data_.dtype(), pybind11::array::ShapeContainer{dimensions.z, dimensions.y, dimensions.x,
                                                           getDataFormat()->getComponents()});
        dims_ = dimensions;

        resource::add(resource::toPY(data_), Resource{.dims = glm::size4_t{dims_, 0},
                                                      .format = format(data_)->getId(),
                                                      .desc = "VolumePY",
                                                      .meta = resource::getMeta(old)});
    }
}

const size3_t& VolumePy::getDimensions() const { return dims_; }

void VolumePy::setSwizzleMask(const SwizzleMask& mask) { swizzleMask_ = mask; }
SwizzleMask VolumePy::getSwizzleMask() const { return swizzleMask_; }

void VolumePy::setInterpolation(InterpolationType interpolation) { interpolation_ = interpolation; }
InterpolationType VolumePy::getInterpolation() const { return interpolation_; }

void VolumePy::setWrapping(const Wrapping3D& wrapping) { wrapping_ = wrapping; }
Wrapping3D VolumePy::getWrapping() const { return wrapping_; }

void VolumePy::updateResource(const ResourceMeta& meta) const {
    resource::meta(resource::toPY(data_), meta);
}

std::shared_ptr<VolumePy> VolumeRAM2PyConverter::createFrom(
    std::shared_ptr<const VolumeRAM> volumeSrc) const {

    pybind11::array data = volumeSrc->dispatch<pybind11::array>([](auto vr) {
        using ValueType = util::PrecisionValueType<decltype(vr)>;
        using CompType = typename util::value_type<ValueType>::type;
        constexpr size_t extent = util::extent<ValueType>::value;

        const auto dims = vr->getDimensions();

        auto shape = extent == 1 ? pybind11::array::ShapeContainer{dims.z, dims.y, dims.x}
                                 : pybind11::array::ShapeContainer{dims.z, dims.y, dims.x, extent};
        pybind11::array_t<CompType> data{shape};

        if (pybind11::array::c_style == (data.flags() & pybind11::array::c_style)) {
            std::memcpy(data.mutable_data(0), vr->getData(), data.nbytes());
        } else {
            throw Exception(
                "Unable to convert from VolumeRAM to VolumePy: numpy array is not C-contiguous.",
                IVW_CONTEXT_CUSTOM("VolumeRAM2PyConverter"));
        }

        return data;
    });

    auto volumeDst = std::make_shared<VolumePy>(
        data, volumeSrc->getSwizzleMask(), volumeSrc->getInterpolation(), volumeSrc->getWrapping());
    return volumeDst;
}

void VolumeRAM2PyConverter::update(std::shared_ptr<const VolumeRAM> volumeSrc,
                                   std::shared_ptr<VolumePy> volumeDst) const {
    volumeDst->setDimensions(volumeSrc->getDimensions());
    volumeDst->setSwizzleMask(volumeSrc->getSwizzleMask());
    volumeDst->setInterpolation(volumeSrc->getInterpolation());
    volumeDst->setWrapping(volumeSrc->getWrapping());

    if (pybind11::array::c_style == (volumeDst->data().flags() & pybind11::array::c_style)) {
        std::memcpy(volumeDst->data().mutable_data(0), volumeSrc->getData(),
                    volumeDst->data().nbytes());
    } else {
        throw Exception(
            "Unable to convert from VolumeRAM to VolumePy: numpy array is not C-contiguous.",
            IVW_CONTEXT);
    }
}

std::shared_ptr<VolumeRAM> VolumePy2RAMConverter::createFrom(
    std::shared_ptr<const VolumePy> volumeSrc) const {

    auto volumeDst = createVolumeRAM(volumeSrc->getDimensions(), volumeSrc->getDataFormat(),
                                     nullptr, volumeSrc->getSwizzleMask(),
                                     volumeSrc->getInterpolation(), volumeSrc->getWrapping());

    auto dst = volumeDst->getData();
    auto src = volumeSrc->data().data(0);
    auto size = volumeSrc->data().nbytes();

    if (pybind11::array::c_style == (volumeSrc->data().flags() & pybind11::array::c_style)) {
        std::memcpy(dst, src, size);
    } else {
        throw Exception(
            "Unable to convert from VolumePy to VolumeRAM: numpy array is not C-contiguous.",
            IVW_CONTEXT);
    }

    return volumeDst;
}

void VolumePy2RAMConverter::update(std::shared_ptr<const VolumePy> volumeSrc,
                                   std::shared_ptr<VolumeRAM> volumeDst) const {
    volumeDst->setDimensions(volumeSrc->getDimensions());
    volumeDst->setSwizzleMask(volumeSrc->getSwizzleMask());
    volumeDst->setInterpolation(volumeSrc->getInterpolation());
    volumeDst->setWrapping(volumeSrc->getWrapping());

    auto dst = volumeDst->getData();
    auto src = volumeSrc->data().data(0);
    auto size = volumeSrc->data().nbytes();

    if (pybind11::array::c_style == (volumeSrc->data().flags() & pybind11::array::c_style)) {
        std::memcpy(dst, src, size);
    } else {
        throw Exception(
            "Unable to convert from VolumePy to VolumeRAM: numpy array is not C-contiguous.",
            IVW_CONTEXT);
    }
}

}  // namespace inviwo
