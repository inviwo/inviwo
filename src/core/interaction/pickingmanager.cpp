/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

PickingManager* PickingManager::instance_ = nullptr;

PickingManager::PickingManager() = default;

PickingManager::~PickingManager() = default;

PickingAction* PickingManager::registerPickingAction(Processor* processor,
                                                     PickingAction::Callback action, size_t size) {
    PickingAction* pickObj = nullptr;

    // Find the smallest object with capacity >= size
    auto it = std::lower_bound(unusedObjects_.begin(), unusedObjects_.end(), size,
                               [](PickingAction* p, size_t s) { return p->getCapacity() < s; });

    if (it != unusedObjects_.end()) {
        pickObj = *it;
        unusedObjects_.erase(it);
        pickObj->setSize(size);
    }

    if (!pickObj) {
        pickingActions_.push_back(std::make_unique<PickingAction>(lastIndex_, size));
        lastIndex_ += size;
        // we can only differentiate up to 2^24-1 picking IDs due to the use of u8vec3 for picking
        // colors
        if (lastIndex_ >= (1 << 24)) {
            LogWarn("More than " << (1 << 24)
                                 << " picking IDs in use. Unreliable picking behavior expected.");
        }
        pickObj = pickingActions_.back().get();
    }
    pickObj->setAction(std::move(action));
    pickObj->setProcessor(processor);
    return pickObj;
}

bool PickingManager::unregisterPickingAction(const PickingAction* p) {
    auto it1 = std::find(unusedObjects_.begin(), unusedObjects_.end(), p);
    if (it1 == unusedObjects_.end()) {

        auto it2 = util::find_if(
            pickingActions_, [p](const std::unique_ptr<PickingAction>& o) { return p == o.get(); });

        if (!pickingActions_.empty() && it2 == pickingActions_.end() - 1) {
            // unregistering the last picking action, don't put it into unused and perform clean-up
            lastIndex_ -= (*it2)->getCapacity();
            pickingActions_.pop_back();

            // clean-up unused queue
            while (!pickingActions_.empty()) {
                auto it = std::find(unusedObjects_.begin(), unusedObjects_.end(),
                                    pickingActions_.back().get());
                if (it == unusedObjects_.end()) {
                    break;
                }
                unusedObjects_.erase(it);
                lastIndex_ -= pickingActions_.back()->getCapacity();
                pickingActions_.pop_back();
            }
            return true;
        } else if (it2 != pickingActions_.end()) {
            (*it2)->setAction(nullptr);
            (*it2)->setProcessor(nullptr);

            auto insit = std::upper_bound(unusedObjects_.begin(), unusedObjects_.end(),
                                          (*it2)->getCapacity(),
                                          [](const size_t& capacity, PickingAction* po) {
                                              return capacity < po->getCapacity();
                                          });

            unusedObjects_.insert(insit, (*it2).get());
            return true;
        }
    }

    return false;
}

auto PickingManager::getPickingActionFromIndex(size_t index) -> Result {
    if (index == 0) return {index, nullptr};

    // This will find the first picking object with an start greater then index.
    auto pIt = std::upper_bound(pickingActions_.begin(), pickingActions_.end(), index,
                                [](const size_t& i, const std::unique_ptr<PickingAction>& p) {
                                    return i < p->getPickingId(0);
                                });

    if (std::distance(pickingActions_.begin(), pIt) > 0) {
        auto po = (*(--pIt)).get();
        if (po->isIndex(index)) {
            return {index, po};
        }
    }
    return {index, nullptr};
}

auto PickingManager::getPickingActionFromColor(const uvec3& c) -> Result {
    return getPickingActionFromIndex(colorToIndex(c));
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

bool PickingManager::isPickingActionRegistered(const PickingAction* action) const {
    return std::any_of(pickingActions_.begin(), pickingActions_.end(),
                       [&](auto& item) { return item.get() == action; });
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
    std::uint32_t index = static_cast<std::uint32_t>(id);

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
    const std::uint32_t r = reverse(static_cast<std::uint8_t>(color[0]));
    const std::uint32_t g = reverse(static_cast<std::uint8_t>(color[1]));
    const std::uint32_t b = reverse(static_cast<std::uint8_t>(color[2]));

    std::uint32_t index = 0;
    for (int i = 0; i < 8; ++i) {
        index |= (((b & (1 << i)) << (0 + 2 * i)));
        index |= (((g & (1 << i)) << (1 + 2 * i)));
        index |= (((r & (1 << i)) << (2 + 2 * i)));
    }
    return index;
}

}  // namespace inviwo
