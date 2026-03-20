/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2026 Inviwo Foundation
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

#include <modules/plotting/interaction/boxselection.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/interaction/events/keyboardkeys.h>
#include <inviwo/core/interaction/events/mousebuttons.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/interactionhandler.h>
#include <inviwo/core/util/dispatcher.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/plotting/datastructures/boxselectiondata.h>
#include <modules/plotting/properties/boxselectionproperty.h>

#include <flags/flags.h>
#include <glm/common.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/vec2.hpp>

namespace inviwo::plot {

namespace {

AxisRangeInteraction getInteraction(BoxSelectionData::Mode mode) {
    using enum BoxSelectionData::Mode;
    switch (mode) {
        case Selection:
            return AxisRangeInteraction::Selection;
        case Filtering:
            return AxisRangeInteraction::Filtering;
        case None:
        default:
            return AxisRangeInteraction::None;
    }
}

AxisRangeInteractionMode getInteractionMode(bool append) {
    return append ? AxisRangeInteractionMode::Append : AxisRangeInteractionMode::Replace;
}

}  // namespace

BoxSelection::BoxSelection(const BoxSelectionProperty& boxSelectionSettings,
                           std::function<dvec2(dvec2 p, const size2_t& dims)> screenToData,
                           MouseButton mouseButton, KeyModifiers modifiers)
    : dragRectSettings_{boxSelectionSettings}
    , screenToData_{std::move(screenToData)}
    , mouseBoxSelection_{"mouseBoxSelection",
                         "Box Selection",
                         [this](Event* e, EventProperty::State state) { handleEvent(e, state); },
                         mouseButton,
                         MouseState::Move | MouseState::Press | MouseState::Release,
                         modifiers,
                         ModifierMatchingBehavior::PartialMatch} {}

void BoxSelection::handleEvent(Event* e, EventProperty::State state) {
    if (!e) return;

    const KeyModifier appendModifier = KeyModifier::Shift;
    KeyModifiers matcherModifiers{flags::none};
    if (const auto* matcher =
            dynamic_cast<MouseEventMatcher*>(mouseBoxSelection_.getEventMatcher())) {
        matcherModifiers = matcher->modifiers();
    }

    if (state == EventProperty::State::Finished) {
        if (const auto* ke = e->getAs<KeyboardEvent>()) {
            if (!match(ModifierMatchingBehavior::PartialMatch, matcherModifiers, ke->modifiers())) {
                // modifier changed
                dragRect_ = std::nullopt;
                dataRect_ = std::nullopt;
                eventCallback_.invoke(AxisRangeEventState::Finished,
                                      getInteraction(dragRectSettings_.mode_),
                                      AxisRangeInteractionMode::Clear, std::nullopt);
                e->setUsed(true);
            } else {
                // selection modifiers still match, but append modifier may have changed
                eventCallback_.invoke(
                    AxisRangeEventState::Updated, getInteraction(dragRectSettings_.mode_),
                    getInteractionMode(ke->modifiers().contains(appendModifier)), dataRect_);
            }
            return;
        }
    }
    auto* me = e->getAs<MouseEvent>();
    if (!me) {
        return;
    }

    if (state == EventProperty::State::Active) {
        const auto interaction = getInteraction(dragRectSettings_.mode_);
        const bool append = me->modifiers().contains(appendModifier);

        if (me->state() == MouseState::Press) {
            dragRect_ = {dvec2{me->pos().x, me->pos().y}, dvec2{me->pos().x, me->pos().y}};
            eventCallback_.invoke(AxisRangeEventState::Started, interaction,
                                  AxisRangeInteractionMode::None, std::nullopt);
            me->setUsed(true);
        } else if (me->state() == MouseState::Release) {
            if (dragRect_ && glm::compMax(glm::abs(me->pos() - (*dragRect_)[0])) <= 1) {
                // Click in empty space
                eventCallback_.invoke(AxisRangeEventState::Finished, interaction,
                                      AxisRangeInteractionMode::Clear, std::nullopt);
            } else {
                eventCallback_.invoke(AxisRangeEventState::Finished, interaction,
                                      AxisRangeInteractionMode::None, std::nullopt);
            }
            dragRect_ = std::nullopt;
            dataRect_ = std::nullopt;
            me->setUsed(true);
        } else if (dragRect_ && (me->state() == MouseState::Move)) {
            // convert position from screen coords to data range
            (*dragRect_)[1] = dvec2{me->pos().x, me->pos().y};
            const auto dstart = screenToData_((*dragRect_)[0], me->canvasSize());
            const auto dend = screenToData_((*dragRect_)[1], me->canvasSize());
            dataRect_ = std::array<dvec2, 2>{glm::min(dstart, dend), glm::max(dstart, dend)};
            eventCallback_.invoke(AxisRangeEventState::Updated, interaction,
                                  getInteractionMode(append), dataRect_);
            me->setUsed(true);
        }

    } else if (state == EventProperty::State::Finished) {
        if (dragRect_) {
            // abort ongoing box selection
            dragRect_ = std::nullopt;
            dataRect_ = std::nullopt;
            eventCallback_.invoke(AxisRangeEventState::Finished,
                                  getInteraction(dragRectSettings_.mode_),
                                  AxisRangeInteractionMode::Clear, std::nullopt);
        }
    }
}

auto BoxSelection::addEventCallback(std::function<SelectionFunc> callback)
    -> SelectionCallbackHandle {
    return eventCallback_.add(std::move(callback));
}

void BoxSelection::reset() { dragRect_.reset(); }

}  // namespace inviwo::plot
