/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/core/interaction/trackball.h>

#include <inviwo/core/datastructures/image/imageram.h>

namespace inviwo {




ScreenToWorldTransformer::ScreenToWorldTransformer(const CameraBase* camera, Image* screen): camera_(camera), screen_(screen) {

}

vec3 ScreenToWorldTransformer::getNormalizedDeviceFromNormalizedScreen(const vec2& normalizedScreenCoord) const {
    vec3 normalizedDeviceCoordinate;
    if (screen_ && screen_->getDepthLayer() &&
        !(glm::any(glm::lessThan(normalizedScreenCoord, vec2(0))) | glm::any(glm::greaterThan(normalizedScreenCoord, vec2(1.f)))) ) {
        const LayerRAM* depthLayer = screen_->getDepthLayer()->getRepresentation<LayerRAM>();
        uvec2 screenPos(normalizedScreenCoord*vec2(vec2(screen_->getDimensions()) - 1.f) + 0.5f);

        double depth = depthLayer->getValueAsSingleDouble(screenPos);
        // Set the depth of the focus point if the user did not click on an object
        // depth==1 is the far plane
        if (depth >= 1.0) {
            // Use focus point for depth
            vec4 lookToClipCoord = camera_->projectionMatrix()*camera_->viewMatrix()*vec4(camera_->getLookTo(), 1.f);
            normalizedDeviceCoordinate = vec3(2.f*normalizedScreenCoord - 1.f, lookToClipCoord.z / lookToClipCoord.w);
        } else {
            //double zNDC = -(depth - 0.5*(camera_->getNearPlaneDist() + camera_->getFarPlaneDist())) /
            //              (0.5*(camera_->getFarPlaneDist() - camera_->getNearPlaneDist()));
            double zNDC = 2.*depth -1.;
            normalizedDeviceCoordinate = vec3(2.f*normalizedScreenCoord - 1.f, static_cast<float>(zNDC));
        }
       

    } else {
        // Default to using focus point for depth
        vec4 lookToClipCoord = camera_->projectionMatrix()*camera_->viewMatrix()*vec4(camera_->getLookTo(), 1.f);

        normalizedDeviceCoordinate = vec3(2.f*normalizedScreenCoord - 1.f, lookToClipCoord.z / lookToClipCoord.w);
    }

    return normalizedDeviceCoordinate;
}

inviwo::vec3 ScreenToWorldTransformer::getWorldPosFromNormalizedDeviceCoords(const vec3& normalizedDeviceCoord) const {

    return camera_->getWorldPosFromNormalizedDeviceCoords(normalizedDeviceCoord);
}

} // namespace