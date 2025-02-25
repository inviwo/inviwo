/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/opactopt/opactoptmoduledefine.h>

#include <string>
#include <map>

#include <modules/opengl/buffer/bufferobject.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/util/glmvec.h>
#include <cmath>
#include <vector>

namespace inviwo {

namespace Approximations {

/**
 * \brief Describes the approximation properties
 */
struct IVW_MODULE_OPACTOPT_API ApproximationProperties {
    std::string name;
    std::string shaderDefineName;
    std::string shaderFile;
    int minCoefficients;
    int maxCoefficients;
    int increment;
};

const std::map<std::string, const ApproximationProperties> approximations{
    {"fourier", {"Fourier", "FOURIER", "opactopt/approximate/fourier.glsl", 1, 31, 1}},
    {"legendre", {"Legendre", "LEGENDRE", "opactopt/approximate/legendre.glsl", 1, 31, 1}},
    {"piecewise", {"Piecewise", "PIECEWISE", "opactopt/approximate/piecewise.glsl", 1, 30, 1}},
    {"powermoments",
     {"Power moments", "POWER_MOMENTS", "opactopt/approximate/powermoments.glsl", 5, 9, 2}},
    {"trigmoments",
     {"Trigonometric moments", "TRIG_MOMENTS", "opactopt/approximate/trigmoments.glsl", 5, 9, 2}}};

double choose(double n, double k);
std::vector<OptionPropertyStringOption> generateApproximationStringOptions();
std::vector<float> generateLegendreCoefficients();

class MomentSettings {
public:
    const float wrapping_zone_angle = 0.1f * M_PI;
    const float overestimation = 0.25;
    MomentSettings();

private:
    BufferObject momentSettingsBuffer_;
    float circleToParameter(float angle, float* pOutMaxParameter);
    void computeWrappingZoneParameters(glm::vec4& p_out_wrapping_zone_parameters,
                                       float new_wrapping_zone_angle);
};


}  // namespace Approximations

}  // namespace inviwo
