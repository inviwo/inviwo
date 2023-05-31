/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <modules/python3/python3moduledefine.h>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/datastructures/image/imagerepresentation.h>
#include <inviwo/core/util/glmvec.h>

#include <memory>
#include <typeindex>

namespace inviwo {

class DataFormatBase;

#include <warn/push>
#include <warn/ignore/attributes>
class IVW_MODULE_PYTHON3_API LayerPy : public LayerRepresentation {
public:
    LayerPy(pybind11::array data, LayerType type = LayerType::Color,
            const SwizzleMask& swizzleMask = swizzlemasks::rgba,
            InterpolationType interpolation = InterpolationType::Linear,
            const Wrapping2D& wrapping = wrapping2d::clampAll);

    LayerPy(size2_t dimensions, LayerType type, const DataFormatBase* format,
            const SwizzleMask& swizzleMask = swizzlemasks::rgba,
            InterpolationType interpolation = InterpolationType::Linear,
            const Wrapping2D& wrapping = wrapping2d::clampAll);

    virtual ~LayerPy();

    LayerPy* clone() const override;
    std::type_index getTypeIndex() const override final;

    virtual void setDimensions(size2_t dimensions) override;
    virtual const size2_t& getDimensions() const override;

    virtual void setSwizzleMask(const SwizzleMask& mask) override;
    virtual SwizzleMask getSwizzleMask() const override;

    virtual void setInterpolation(InterpolationType interpolation) override;
    virtual InterpolationType getInterpolation() const override;

    virtual void setWrapping(const Wrapping2D& wrapping) override;
    virtual Wrapping2D getWrapping() const override;

    virtual bool copyRepresentationsTo(LayerRepresentation*) const override;

    pybind11::array& data() { return data_; }
    const pybind11::array& data() const { return data_; }

private:
    SwizzleMask swizzleMask_;
    InterpolationType interpolation_;
    Wrapping2D wrapping_;

    pybind11::array data_;
    size2_t dims_;
};
#include <warn/pop>

class IVW_MODULE_PYTHON3_API LayerRAM2PyConverter
    : public RepresentationConverterType<LayerRepresentation, LayerRAM, LayerPy> {
public:
    virtual std::shared_ptr<LayerPy> createFrom(
        std::shared_ptr<const LayerRAM> source) const override;
    virtual void update(std::shared_ptr<const LayerRAM> source,
                        std::shared_ptr<LayerPy> destination) const override;
};

class IVW_MODULE_PYTHON3_API LayerPy2RAMConverter
    : public RepresentationConverterType<LayerRepresentation, LayerPy, LayerRAM> {
public:
    virtual std::shared_ptr<LayerRAM> createFrom(
        std::shared_ptr<const LayerPy> source) const override;
    virtual void update(std::shared_ptr<const LayerPy> source,
                        std::shared_ptr<LayerRAM> destination) const override;
};

}  // namespace inviwo
