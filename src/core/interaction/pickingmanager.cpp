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

PickingManager::~PickingManager() {}

const PickingObject* PickingManager::registerPickingCallback(
    std::function<void(const PickingObject*)> callback, size_t size /*= 1*/) {
    PickingObject* pickObj = nullptr;

    // Find the smallest object with capacity >= size
    auto it = std::lower_bound(unusedObjects_.begin(), unusedObjects_.end(), size,
                               [](PickingObject* p, int size) {
                                   return p->getCapacity() < size;
                               });

    if (it != unusedObjects_.end()) {
        pickObj = *it;
        unusedObjects_.erase(it);
        pickObj->setSize(size);
    }

    if (!pickObj) {
        pickingObjects_.push_back(util::make_unique<PickingObject>(lastIndex_, size));
        lastIndex_ += size;
        pickObj = pickingObjects_.back().get();
    }
    pickObj->setCallback(callback);
    return pickObj;
}

bool PickingManager::unregisterPickingObject(const PickingObject* p) {
    auto it = std::find(unusedObjects_.begin(), unusedObjects_.end(), p);
    if (it == unusedObjects_.end()) {
        auto it = util::find_if(
            pickingObjects_, [p](const std::unique_ptr<PickingObject>& o) { return p == o.get(); });

        if (it != pickingObjects_.end()) {
            (*it)->setCallback(nullptr);

            auto insit =
                std::upper_bound(unusedObjects_.begin(), unusedObjects_.end(), (*it)->getCapacity(),
                                 [](const size_t& capacity, PickingObject* p) {
                return capacity < p->getCapacity();
            });

            unusedObjects_.insert(insit, (*it).get());
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

inviwo::uvec3 PickingManager::indexToColor(size_t id) {
    const size_t Nr = 13;
    const size_t colors = 256 * 256 * 256;
    id++; // avoid zero

    size_t i = id % Nr;
    size_t n = static_cast<size_t>(
        std::floor(i * colors / static_cast<double>(Nr) + id / static_cast<double>(Nr)));
    return uvec3(static_cast<unsigned char>(std::floor(n / (256.0 * 256.0))),
              static_cast<unsigned char>(std::floor(static_cast<double>(n) / 256.0)) % 256,
              static_cast<unsigned char>(n % 256));
}

size_t PickingManager::colorToIndex(uvec3 color) {
    const size_t Nr = 13;
    const size_t colors = 256 * 256 * 256;
    size_t n = color.r * 256 * 256 + color.g * 256 + color.b;
    auto i = std::round(n / (colors / static_cast<double>(Nr)));
    auto o = n - std::round(i * colors / static_cast<double>(Nr));
    return static_cast<size_t>(o * Nr + i)-1;
}

PickingObject* PickingManager::getPickingObjectFromColor(const uvec3& c) {
    auto index = colorToIndex(c);

    // This will find the first picking object with an start greater then index.
    auto pit = std::upper_bound(pickingObjects_.begin(), pickingObjects_.end(), index,
                                [](const size_t& index, const std::unique_ptr<PickingObject>& p) {
                                    return index < p->getPickingId(0);
                                });

    if (std::distance(pickingObjects_.begin(), pit) > 0) {
        auto po = (*(--pit)).get();
        const auto start = po->getPickingId(0);
        if (index >= start && index < start + po->getSize()) {
            po->setPickedId(index - start);
            return po;
        }
    }

    return nullptr;
}

}  // namespace
