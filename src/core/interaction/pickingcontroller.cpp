/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <inviwo/core/interaction/pickingcontroller.h>
#include <inviwo/core/interaction/pickingmanager.h>
#include <inviwo/core/interaction/pickingaction.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/interaction/events/eventpropagator.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/pickingstate.h>

namespace inviwo {

PickingController::PickingController() {}

void PickingController::handlePickingEvent(EventPropagator* propagator, Event* event) {
    if (!propagator) return;
    if (!event) return;

    switch (event->hash()) {
        case MouseEvent::chash(): {
            auto e = static_cast<MouseEvent*>(event);
            auto ndc = e->ndc();

            auto coord = glm::clamp(e->pos(), dvec2(0.0), dvec2(e->canvasSize() - uvec2(1)));

            // Toggle states
            if (!state_.mousePressed_ && e->state() == MouseState::Press) {
                state_.mousePressed_ = true;
                state_.pressedPa_ = findPickingAction(coord);
                state_.pressNDC_ = e->ndc();
            } else if (state_.mousePressed_ && e->state() == MouseState::Release) {
                state_.mousePressed_ = false;
                state_.pressedPa_ = {0, nullptr};
                state_.pressNDC_ = dvec3(0.0);
            }

            auto pa = state_.mousePressed_ ? state_.pressedPa_ : findPickingAction(coord);

            if (state_.previousPa_.second && pa.first != state_.previousPa_.first) {
                PickingEvent pickingEvent(state_.previousPa_.second, PickingState::Finished, e,
                                          state_.pressNDC_, state_.previousNDC_,
                                          state_.previousPa_.first);
                propagator->propagateEvent(&pickingEvent, nullptr);
            }

            if (pa.second) {
                auto ps = pa.first == state_.previousPa_.first ? PickingState::Updated
                                                               : PickingState::Started;

                PickingEvent pickingEvent(pa.second, ps, e, state_.pressNDC_, state_.previousNDC_,
                                          pa.first);
                propagator->propagateEvent(&pickingEvent, nullptr);
            }

            state_.previousPa_ = pa;
            state_.previousNDC_ = ndc;

            break;
        }
        case WheelEvent::chash(): {
            auto e = static_cast<WheelEvent*>(event);
            auto coord = glm::clamp(e->pos(), dvec2(0.0), dvec2(e->canvasSize() - uvec2(1)));
            auto pa = findPickingAction(coord);

            break;
        }
    }
}

void PickingController::setPickingSource(const std::shared_ptr<const Image>& src) { src_ = src; }

std::pair<size_t, const PickingAction*> PickingController::findPickingAction(const uvec2& coord) {
    if (src_ && PickingManager::getPtr()->pickingEnabled()) {
        if (auto pickingLayer = src_->getPickingLayer()) {
            const auto pickingLayerRAM = pickingLayer->getRepresentation<LayerRAM>();
            const auto value = pickingLayerRAM->getAsNormalizedDVec4(coord);
            const auto pickedColor = (value.a > 0.0 ? value.rgb() : dvec3(0.0));
            const uvec3 color(pickedColor * 255.0);
            return PickingManager::getPtr()->getPickingActionFromColor(color);
        }
    }
    return {0, nullptr};
}

bool PickingController::pickingEnabled() const { return PickingManager::getPtr()->pickingEnabled(); }

} // namespace

