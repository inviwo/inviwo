/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <inviwo/core/util/settings/camerasettings.h>

namespace inviwo {

CameraSettings::CameraSettings()
    : Settings("Camera Settings")
    , updateNearFar{"updateNearFar", "Update Near/Far Distances On Fit", true}
    , updateLookRanges{"updateLookRanges", "Update Look-to/-from Ranges On Fit", true}
    , fittingRatio{"fittingRatio", "Fitting Ratio", 1.05f, 0, 2, 0.01f}
    , zoomFactor{"zoomFactor", "Zoom Factor",
                 util::ordinalScale(camerautil::defaultZoomFactor, 10000.0f)
                     .setMin(1.0f)
                     .set("The far plane will be placed at 'Zoom Factor' times the largest"
                          " extent of the bounding box"_help)}
    , farNearRatio{"farNearRatio", "Far Near Ratio",
                   util::ordinalScale(camerautil::defaultFarNearRatio, 1.0e6f).setMin(1.0f)} {

    addProperties(updateNearFar, updateLookRanges, fittingRatio, zoomFactor, farNearRatio);

    load();
}

camerautil::UpdateNearFar CameraSettings::getUpdateNearFar() const {
    return updateNearFar ? camerautil::UpdateNearFar::Yes : camerautil::UpdateNearFar::No;
}
camerautil::UpdateLookRanges CameraSettings::getUpdateLookRanges() const {
    return updateLookRanges ? camerautil::UpdateLookRanges::Yes : camerautil::UpdateLookRanges::No;
}

}  // namespace inviwo
