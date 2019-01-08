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

#include <modules/opencl/light/packedlightsource.h>

namespace inviwo {

PackedLightSource baseLightToPackedLight(const LightSource* lightsource, float radianceScale) {
    PackedLightSource light;
    light.tm = lightsource->getCoordinateTransformer().getModelToWorldMatrix();
    light.radiance = vec4(radianceScale * lightsource->getIntensity(), 1.f);
    light.type = static_cast<int>(lightsource->getLightSourceType());
    light.area = lightsource->getArea();
    light.cosFOV = std::cos(glm::radians(lightsource->getFieldOfView() / 2.f));
    light.size = lightsource->getSize();
    return light;
}

PackedLightSource baseLightToPackedLight(const LightSource* lightsource, float radianceScale,
                                         const mat4& transformLightMat) {
    PackedLightSource light;
    light.tm = transformLightMat * lightsource->getCoordinateTransformer().getModelToWorldMatrix();
    light.radiance = vec4(radianceScale * lightsource->getIntensity(), 1.f);
    light.type = static_cast<int>(lightsource->getLightSourceType());
    light.area = lightsource->getArea();
    light.cosFOV = std::cos(glm::radians(lightsource->getFieldOfView() / 2.f));
    // Transform width and height.
    mat4 invTransform = glm::inverse(light.tm);
    vec4 transformedWidth = invTransform * vec4(lightsource->getSize().x, 0.f, 0.f, 0.f);
    vec4 transformedHeight = invTransform * vec4(0.f, lightsource->getSize().y, 0.f, 0.f);
    light.size.x = glm::length(vec3(transformedWidth));
    light.size.y = glm::length(vec3(transformedHeight));

    return light;
}

}  // namespace inviwo
