/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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
#include <inviwo/core/properties/ordinalrefproperty.h>

#include <memory>

namespace inviwo {

class CameraProperty;

namespace util {

IVW_CORE_API FloatRefProperty* getCameraFovProperty(CameraProperty* comp);

IVW_CORE_API std::unique_ptr<FloatRefProperty> createCameraFovProperty(
    std::function<float()> get, std::function<void(const float&)> set);

IVW_CORE_API FloatRefProperty* updateOrCreateCameraFovProperty(
    CameraProperty* comp, std::function<float()> get, std::function<void(const float&)> set);


IVW_CORE_API FloatRefProperty* getCameraWidthProperty(CameraProperty* comp);

IVW_CORE_API std::unique_ptr<FloatRefProperty> createCameraWidthProperty(
    std::function<float()> get, std::function<void(const float&)> set);

IVW_CORE_API FloatRefProperty* updateOrCreateCameraWidthProperty(
    CameraProperty* comp, std::function<float()> get, std::function<void(const float&)> set);



IVW_CORE_API FloatVec2RefProperty* getCameraSeperationProperty(CameraProperty* comp);

IVW_CORE_API std::unique_ptr<FloatVec2RefProperty> createCameraSeperationProperty(
    std::function<vec2()> get, std::function<void(const vec2&)> set);

IVW_CORE_API FloatVec2RefProperty* updateOrCreateCameraSeperationProperty(
    CameraProperty* comp, std::function<vec2()> get, std::function<void(const vec2&)> set);

}  // namespace util

}  // namespace inviwo
