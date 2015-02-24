/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/core/interaction/pickingmanager.h>
#include <inviwo/core/util/colorconversion.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/systemsettings.h>

namespace inviwo {

PickingManager::~PickingManager() {
    for (auto& elem : pickingObjects_) delete elem;
}

bool PickingManager::unregisterPickingObject(const PickingObject* p) {
    std::vector<PickingObject*>::iterator it =
        std::find(unRegisteredPickingObjects_.begin(), unRegisteredPickingObjects_.end(), p);

    if (it == unRegisteredPickingObjects_.end()) {
        it = std::find(pickingObjects_.begin(), pickingObjects_.end(), p);

        if (it != pickingObjects_.end()) {
            unRegisteredPickingObjects_.push_back(*it);
            return true;
        }
    }

    return false;
}

bool PickingManager::pickingEnabled() {
    BoolProperty* pickingEnabledProperty = dynamic_cast<BoolProperty*>(
        InviwoApplication::getPtr()->getSettingsByType<SystemSettings>()->getPropertyByIdentifier(
            "enablePicking"));
    return (pickingEnabledProperty && pickingEnabledProperty->get());
}

PickingObject* PickingManager::getPickingObjectFromColor(const DataVec3UINT8::type& c) {
    std::vector<PickingObject*>::iterator it =
        std::find_if(pickingObjects_.begin(), pickingObjects_.end(), FindPickingObject(c));

    if (it != pickingObjects_.end()) return (*it);

    return nullptr;
}

PickingObject* PickingManager::generatePickingObject(size_t id) {
    float idF = static_cast<float>(id);
    // Hue /Saturation / Value
    // Hue is based on Golden Ratio for unique and distinct color differences.
    float valueDiff = 0.05f * std::floor(idF / 100.f);

    if (valueDiff > 0.7f) {
        LogError("Maximum number of picking colors reached at ID : " << id);
        return nullptr;
    }

    vec3 hsv = vec3(idF * M_PI - floor(idF * M_PI), 0.5f, 0.95f - valueDiff);
    dvec3 rgb = dvec3(hsv2rgb(hsv));
    DataVec3UINT8::type rgbUINT8;
    DataVec3UINT8::get()->vec3DoubleToValue(rgb * 255.0, &rgbUINT8);
    return new PickingObject(id, rgbUINT8);
}

void PickingManager::performUniqueColorGenerationTest(int iterations) {
    std::vector<DataVec3UINT8::type> colorVec;
    bool passed = true;

    for (int i = 0; i < iterations; i++) {
        float idF = static_cast<float>(i);
        float valueDiff = 0.05f * std::floor(idF / 100.f);

        if (valueDiff > 0.85f) {
            LogError("Maximum number of picking colors reached at ID : " << i);
            return;
        }

        vec3 hsv = vec3(idF * M_PI - floor(idF * M_PI), 0.5f, 0.95f - valueDiff);
        dvec3 rgb = dvec3(hsv2rgb(hsv));
        DataVec3UINT8::type rgbUINT8;
        DataVec3UINT8::get()->vec3DoubleToValue(rgb * 255.0, &rgbUINT8);

        if (std::find(colorVec.begin(), colorVec.end(), rgbUINT8) != colorVec.end()) {
            ivec3 ic = ivec3(rgbUINT8.x, rgbUINT8.y, rgbUINT8.z);
            LogInfo("Duplicate Picking Color : (" << ic.x << "," << ic.y << "," << ic.z
                                                  << ") at iteration " << i << " with valueDiff "
                                                  << valueDiff);
            passed = false;
        } else
            colorVec.push_back(rgbUINT8);
    }

    if (passed)
        LogInfo("performUniqueColorGenerationTest passed with " << iterations << " iterations");
}

}  // namespace
