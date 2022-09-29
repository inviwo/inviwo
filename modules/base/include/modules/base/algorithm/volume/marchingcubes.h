/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2022 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/util/glmvec.h>        // for size3_t, vec4

#include <functional>                       // for function
#include <memory>                           // for shared_ptr

namespace inviwo {
class Mesh;
class Volume;

namespace util {

/**
 * Extracts an isosurface from a volume using the Marching Cubes algorithm
 *
 * Note: Share interface with util::marchingtetrahedron
 *
 * @param volume the scalar volume
 * @param iso iso-value for the extracted surface
 * @param color the color of the resulting surface
 * @param invert flips the normals of the surface normals (useful when values greater than the
 * iso-value is 'outside' of the surface)
 * @param enclose whether to create surface where the isosurface intersects the volume boundaries
 * @param progressCallback if set, will be called will executing with the current progress in the
 * interval [0,1], usefull for progressbars
 * @param maskingCallback optional callback to test whether current cell should be evaluated or not
 * (return true to include current cell)
 */

IVW_MODULE_BASE_API std::shared_ptr<Mesh> marchingcubes(
    std::shared_ptr<const Volume> volume, double iso, const vec4& color, bool invert, bool enclose,
    std::function<void(float)> progressCallback = std::function<void(float)>(),
    std::function<bool(const size3_t&)> maskingCallback = [](const size3_t&) { return true; });
}  // namespace util

}  // namespace inviwo
