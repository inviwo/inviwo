/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2016 Inviwo Foundation
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

PickingManager::~PickingManager() = default;

PickingObject* PickingManager::registerPickingCallback(PickingObject::Action action, size_t size) {
    PickingObject* pickObj = nullptr;

    // Find the smallest object with capacity >= size
    auto it = std::lower_bound(unusedObjects_.begin(), unusedObjects_.end(), size,
                               [](PickingObject* p, size_t s) { return p->getCapacity() < s; });

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
    pickObj->setAction(std::move(action));
    return pickObj;
}

bool PickingManager::unregisterPickingObject(const PickingObject* p) {
    auto it1 = std::find(unusedObjects_.begin(), unusedObjects_.end(), p);
    if (it1 == unusedObjects_.end()) {
        auto it2 = util::find_if(
            pickingObjects_, [p](const std::unique_ptr<PickingObject>& o) { return p == o.get(); });

        if (it2 != pickingObjects_.end()) {
            (*it2)->setAction(nullptr);

            auto insit = std::upper_bound(unusedObjects_.begin(), unusedObjects_.end(),
                                          (*it2)->getCapacity(),
                                          [](const size_t& capacity, PickingObject* po) {
                                              return capacity < po->getCapacity();
                                          });

            unusedObjects_.insert(insit, (*it2).get());
            return true;
        }
    }

    return false;
}

bool PickingManager::pickingEnabled() {
    if (!enableCallback_) {
        auto picking = &(InviwoApplication::getPtr()
                             ->getSettingsByType<SystemSettings>()
                             ->enablePickingProperty_);

        enableCallback_ = picking->onChange([this, picking]() { enabled_ = picking->get(); });
        enabled_ = picking->get();
    }

    return enabled_;
}

// First the left four bits are swapped with the right four bits.
// Then all adjacent pairs are swapped and then all adjacent single bits.
// This results in a reversed order.
std::uint8_t reverse(std::uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

uvec3 PickingManager::indexToColor(size_t id) {
    std::uint32_t index = static_cast<std::uint32_t>(id + 1);

    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;

    for (int i = 0; i < 8; ++i) {
        r |= ((index & (1 << (3 * i + 2))) >> (2 * i + 2));
        g |= ((index & (1 << (3 * i + 1))) >> (2 * i + 1));
        b |= ((index & (1 << (3 * i + 0))) >> (2 * i + 0));
    }

    return uvec3{reverse(r), reverse(g), reverse(b)};
}

size_t PickingManager::colorToIndex(uvec3 color) {
    const std::uint32_t r = reverse(color[0]);
    const std::uint32_t g = reverse(color[1]);
    const std::uint32_t b = reverse(color[2]);

    std::uint32_t index = 0;
    for (int i = 0; i < 8; ++i) {
        index |= (((b & (1 << i)) << (0 + 2 * i)));
        index |= (((g & (1 << i)) << (1 + 2 * i)));
        index |= (((r & (1 << i)) << (2 + 2 * i)));
    }
    return index - 1;
}

PickingObject* PickingManager::getPickingObjectFromColor(const uvec3& c) {
    auto index = colorToIndex(c);

    // This will find the first picking object with an start greater then index.
    auto pit = std::upper_bound(pickingObjects_.begin(), pickingObjects_.end(), index,
                                [](const size_t& i, const std::unique_ptr<PickingObject>& p) {
                                    return i < p->getPickingId(0);
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
