/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>

#include <memory>
#include <vector>
#include <utility>

namespace inviwo {

class Mesh;
class CameraProperty;

namespace camerautil {

enum class Side { XNegative, XPositive, YNegative, YPositive, ZNegative, ZPositive };
enum class UpdateNearFar { Yes, No };
enum class UpdateLookRanges { Yes, No };

/**
 * Setup the camera parameters such that the whole boundingBox spaned by basis and offset
 * will be inside the view frustum, for the given view direction
 *
 * @param cam       the Camera to update
 * @param boundingBox the basis and offset of the bounding box that will fit inside the new
 *                    view frustum
 * @param viewDir    the view direction of when viewing the bounding box
 * @param lookDir    the up direction of when viewing the bounding box
 * @param fitRatio   determines the spacing between volume and the boundaries of the view frustum. A
 *                   fit ratio of 1 means a perfect fit, no space between frustum and volume. The
 *                   aspect ratio is taken into account.
 * @param updateNearFar the camera's new/far clip ranges are updated if Yes @see setCameraNearFar
 * @param updateLookRanges   the camera's look-to/look-from ranges are updated if Yes @see
 * setCameraLookRanges
 */
IVW_CORE_API void setCameraView(CameraProperty &cam, const mat4 &boundingBox, vec3 viewDir,
                                vec3 lookUp, float fitRatio = 1.05f,
                                UpdateNearFar updateNearFar = UpdateNearFar::No,
                                UpdateLookRanges updateLookRanges = UpdateLookRanges::No);

/**
 * Setup the camera parameters such that the whole boundingBox spaned by basis and offset
 * will be inside the view frustum, using the current view direction
 *
 * @param cam       the Camera to update
 * @param boundingBox the basis and offset of the bounding box that will fit inside the new
 *                    view frustum
 * @param fitRatio   determines the spacing between volume and the boundaries of the view frustum. A
 *                   fit ratio of 1 means a perfect fit, no space between frustum and volume. The
 *                   aspect ratio is taken into account.
 * @param updateNearFar the camera's new/far clip ranges are updated if Yes @see setCameraNearFar
 * @param updateLookRanges   the camera's look-to/look-from ranges are updated if Yes @see
 * setCameraLookRanges
 */
IVW_CORE_API void setCameraView(CameraProperty &cam, const mat4 &boundingBox,
                                float fitRatio = 1.05f,
                                UpdateNearFar updateNearFar = UpdateNearFar::No,
                                UpdateLookRanges updateLookRanges = UpdateLookRanges::No);

/**
 * Setup the camera parameters such that the whole boundingBox spaned by basis and offset
 * will be inside the view frustum.
 *
 * @param cam       the Camera to update
 * @param boundingBox the basis and offset of the bounding box that will fit inside the new
 * view frustum
 * @param side       this side of the bounding box will be facing the camera afterward
 * @param fitRatio   determines the spacing between volume and the boundaries of the view frustum. A
 *                   fit ratio of 1 means a perfect fit, no space between frustum and volume. The
 * aspect ratio is taken into account.
 * @param updateNearFar the camera's new/far clip ranges are updated if Yes @see setCameraNearFar
 * @param updateLookRanges   the camera's look-to/look-from ranges are updated if Yes @see
 * setCameraLookRanges
 */
IVW_CORE_API void setCameraView(CameraProperty &cam, const mat4 &boundingBox, Side side,
                                float fitRatio = 1.05f,
                                UpdateNearFar updateNearFar = UpdateNearFar::No,
                                UpdateLookRanges updateLookRanges = UpdateLookRanges::No);

/**
 * Set the ranges of the look to and look from properties of the camera. Will center around the mid
 * point of dataToWorld. The lookTo will be set to ranges to stay within the volume. The lookFrom
 * will be set to the mid point +- the basis vector of boundingBox times the zoom factor. That is
 * a zoom factor of 25 will allow to zoom out to a distance of "25 volumes".
 *
 * @param cam           camera to update
 * @param boundingBox  basis and offset of the bounding box used to determinte the ranges
 * @param maxZoomFactor determines how far away from the volume the user will be able to zoom out.
 */
IVW_CORE_API void setCameraLookRanges(CameraProperty &cam, const mat4 &boundingBox,
                                      float maxZoomFactor = 25.f);

/**
 * Computes appropriate near and far clip distances for the given bounding box and zoom factor.
 * Makes sure that the far plane is distant enough to avoid clipping given to current zoomfactor.
 * @see setCameraLookRanges
 */
IVW_CORE_API std::pair<float, float> computeCameraNearFar(const mat4 &boundingBox,
                                                          float maxZoomFactor = 25.f,
                                                          float nearFarRatio = 1.f / 10000.f);

/**
 * Sets the near and far clip distances of the camera based on the given bounding volume and zoom
 * factor. Ensures that the far plane is distant enough to avoid clipping given to current
 * zoomfactor.
 * @see computeCameraNearFar
 * @see setCameraLookRanges
 */
IVW_CORE_API void setCameraNearFar(CameraProperty &cam, const mat4 &boundingBox,
                                   float maxZoomFactor = 25.f, float nearFarRatio = 1.f / 10000.f);

}  // namespace camerautil

}  // namespace inviwo
