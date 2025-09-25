/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2025 Inviwo Foundation
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

#include <modules/python3/python3moduledefine.h>  // for IVW_MODULE_PYTHON3_API

#include <pybind11/pybind11.h>  // IWYU pragma: keep
#include <pybind11/numpy.h>     // for array

#include <inviwo/core/datastructures/image/imagetypes.h>             // for InterpolationType
#include <inviwo/core/datastructures/representationconverter.h>      // for RepresentationConver...
#include <inviwo/core/datastructures/volume/volumeram.h>             // for VolumeRAM
#include <inviwo/core/datastructures/volume/volumerepresentation.h>  // for VolumeRepresentation
#include <inviwo/core/util/glmvec.h>                                 // for size3_t

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <memory>     // for shared_ptr
#include <typeindex>  // for type_index
#include <optional>

namespace inviwo {

class DataFormatBase;

#include <warn/push>
#include <warn/ignore/attributes>
/**
 * \ingroup datastructures
 */
class IVW_MODULE_PYTHON3_API VolumePy : public VolumeRepresentation {
public:
    VolumePy(pybind11::array data,
             const SwizzleMask& swizzleMask = VolumeConfig::defaultSwizzleMask,
             InterpolationType interpolation = VolumeConfig::defaultInterpolation,
             const Wrapping3D& wrapping = VolumeConfig::defaultWrapping);

    VolumePy(size3_t dimensions, const DataFormatBase* format,
             const SwizzleMask& swizzleMask = VolumeConfig::defaultSwizzleMask,
             InterpolationType interpolation = VolumeConfig::defaultInterpolation,
             const Wrapping3D& wrapping = VolumeConfig::defaultWrapping);

    explicit VolumePy(const VolumeReprConfig& config);
    VolumePy(VolumePy&&) = delete;
    VolumePy& operator=(const VolumePy&) = delete;
    VolumePy& operator=(VolumePy&&) = delete;
    virtual ~VolumePy();

    VolumePy* clone() const override;
    std::type_index getTypeIndex() const override;

    virtual const DataFormatBase* getDataFormat() const override;

    virtual void setDimensions(size3_t dimensions) override;
    virtual const size3_t& getDimensions() const override;

    virtual void setSwizzleMask(const SwizzleMask& mask) override;
    virtual SwizzleMask getSwizzleMask() const override;

    virtual void setInterpolation(InterpolationType interpolation) override;
    virtual InterpolationType getInterpolation() const override;

    virtual void setWrapping(const Wrapping3D& wrapping) override;
    virtual Wrapping3D getWrapping() const override;

    pybind11::array& data() { return data_; }
    const pybind11::array& data() const { return data_; }

    virtual void updateResource(const ResourceMeta& meta) const override;

private:
    VolumePy(const VolumePy&);

    std::optional<pybind11::gil_scoped_acquire> gil_;
    SwizzleMask swizzleMask_;
    InterpolationType interpolation_;
    Wrapping3D wrapping_;

    pybind11::array data_;
    size3_t dims_;
};
#include <warn/pop>

class IVW_MODULE_PYTHON3_API VolumeRAM2PyConverter
    : public RepresentationConverterType<VolumeRepresentation, VolumeRAM, VolumePy> {
public:
    virtual std::shared_ptr<VolumePy> createFrom(
        std::shared_ptr<const VolumeRAM> volumeSrc) const override;
    virtual void update(std::shared_ptr<const VolumeRAM> volumeSrc,
                        std::shared_ptr<VolumePy> volumeDst) const override;
};

class IVW_MODULE_PYTHON3_API VolumePy2RAMConverter
    : public RepresentationConverterType<VolumeRepresentation, VolumePy, VolumeRAM> {
public:
    virtual std::shared_ptr<VolumeRAM> createFrom(
        std::shared_ptr<const VolumePy> volumeSrc) const override;
    virtual void update(std::shared_ptr<const VolumePy> volumeSrc,
                        std::shared_ptr<VolumeRAM> volumeDst) const override;
};

}  // namespace inviwo
