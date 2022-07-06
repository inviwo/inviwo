/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2021 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {

/**
 * \ingroup properties
 * \class RaycastingProperty
 * \brief composite property holding parameters for volume raycasting
 */
class IVW_CORE_API RaycastingProperty : public CompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    enum class RenderingType { Dvr, DvrIsosurface, Isosurface };
    enum class Classification { None, TF, Voxel };
    enum class CompositingType {
        Dvr,
        MaximumIntensity,
        FirstHitPoints,
        FirstHitNormals,
        FirstHistNormalsView,
        FirstHitDepth
    };
    enum class GradientComputation {
        None,
        Forward,
        Backward,
        Central,
        CentralHigherOrder,
        PrecomputedXYZ,
        PrecomputedYZW
    };

    RaycastingProperty(std::string_view identifier, std::string_view displayName,
                       InvalidationLevel = InvalidationLevel::InvalidResources,
                       PropertySemantics semantics = PropertySemantics::Default);

    RaycastingProperty(const RaycastingProperty& rhs);
    virtual ~RaycastingProperty() = default;

    virtual RaycastingProperty* clone() const override;

    OptionProperty<RenderingType> renderingType_;
    OptionProperty<Classification> classification_;
    OptionProperty<CompositingType> compositing_;
    OptionProperty<GradientComputation> gradientComputation_;

    FloatProperty samplingRate_;
};

}  // namespace inviwo
