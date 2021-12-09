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

#include <inviwo/core/properties/raycastingproperty.h>

namespace inviwo {

const std::string RaycastingProperty::classIdentifier = "org.inviwo.RaycastingProperty";
std::string RaycastingProperty::getClassIdentifier() const { return classIdentifier; }

RaycastingProperty::RaycastingProperty(std::string_view identifier, std::string_view displayName,
                                       InvalidationLevel invalidationLevel,
                                       PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , renderingType_("renderingType", "Rendering",
                     {{"dvr", "Direct Volume Rendering", RenderingType::Dvr},
                      {"dvriso", "DVR + Isosurfaces", RenderingType::DvrIsosurface},
                      {"iso", "Isosurfaces", RenderingType::Isosurface}},
                     0, InvalidationLevel::InvalidResources)
    , classification_("classificationMode", "Classification",
                      {{"none", "None", Classification::None},
                       {"transfer-function", "Transfer Function", Classification::TF},
                       {"voxel-value", "Voxel Value", Classification::Voxel}},
                      1, InvalidationLevel::InvalidResources)
    , compositing_(
          "compositingMode", "Compositing",
          {{"dvr", "Direct Volume Rendering", CompositingType::Dvr},
           {"mip", "Maximum Intensity Projection (MIP)", CompositingType::MaximumIntensity},
           {"fhp", "First Hit (Points)", CompositingType::FirstHitPoints},
           {"fhn", "First Hit (Normals)", CompositingType::FirstHitNormals},
           {"fhnvs", "First Hit (View Space Normals)", CompositingType::FirstHistNormalsView},
           {"fhd", "First Hit (Depth)", CompositingType::FirstHitDepth},
           {"fhp", "First Hit (Points)", CompositingType::FirstHitPoints}},
          0, InvalidationLevel::InvalidResources)
    , gradientComputation_(
          "gradientComputationMode", "Gradient",
          {{"none", "None", GradientComputation::None},
           {"forward", "Forward Differences", GradientComputation::Forward},
           {"backward", "Backward Differences", GradientComputation::Backward},
           {"central", "Central Differences", GradientComputation::Central},
           {"central-higher", "Higher-order Central Differences",
            GradientComputation::CentralHigherOrder},
           {"precomputedXYZ", "Pre-computed Gradients (xyz)", GradientComputation::PrecomputedXYZ},
           {"precomputedYZW", "Pre-computed Gradients (yzw)", GradientComputation::PrecomputedYZW}},
          3, InvalidationLevel::InvalidResources)
    , samplingRate_("samplingRate", "Sampling rate", 2.0f, 1.0f, 20.0f) {

    addProperties(renderingType_, classification_, compositing_, gradientComputation_,
                  samplingRate_);
}

RaycastingProperty::RaycastingProperty(const RaycastingProperty& rhs)
    : CompositeProperty(rhs)
    , renderingType_(rhs.renderingType_)
    , classification_(rhs.classification_)
    , compositing_(rhs.compositing_)
    , gradientComputation_(rhs.gradientComputation_)
    , samplingRate_(rhs.samplingRate_) {

    addProperties(renderingType_, classification_, compositing_, gradientComputation_,
                  samplingRate_);
}

RaycastingProperty* RaycastingProperty::clone() const { return new RaycastingProperty(*this); }

}  // namespace inviwo
