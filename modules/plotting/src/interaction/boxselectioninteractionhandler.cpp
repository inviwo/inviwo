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

#include <modules/plotting/interaction/boxselectioninteractionhandler.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/interaction/events/keyboardkeys.h>
#include <inviwo/core/interaction/events/mousebuttons.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/interactionhandler.h>
#include <inviwo/core/util/dispatcher.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/plotting/datastructures/boxselection.h>
#include <modules/plotting/properties/boxselectionproperty.h>

#include <flags/flags.h>
#include <glm/common.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/vec2.hpp>

namespace inviwo::plot {

BoxSelectionInteractionHandler::BoxSelectionInteractionHandler(
    const BoxSelectionProperty& boxSelectionSettings,
    std::function<dvec2(dvec2 p, const size2_t& dims)> screenToData)
    : InteractionHandler{}
    , dragRectSettings_{boxSelectionSettings}
    , screenToData_{std::move(screenToData)}
    , mouseBoxSelection_{"mouseBoxSelection",
                         "Box Selection",
                         [this](Event* e) { mouseEvent(e->getAs<MouseEvent>(), false); },
                         MouseButton::Left,
                         MouseState::Move | MouseState::Press | MouseState::Release,
                         KeyModifier::Shift}
    , mouseBoxSelectionAppend_{"mouseBoxSelectionAppend",
                               "Box Selection Append",
                               [this](Event* e) { mouseEvent(e->getAs<MouseEvent>(), true); },
                               MouseButton::Left,
                               MouseState::Move | MouseState::Press | MouseState::Release,
                               KeyModifier::Shift | KeyModifier::Control} {}

void BoxSelectionInteractionHandler::mouseEvent(MouseEvent* me, bool append) {
    if (!me) return;

    const auto interaction = [mode = dragRectSettings_.mode_.get()]() {
        using enum BoxSelection::Mode;
        switch (mode) {
            case Selection:
                return AxisRangeInteraction::Selection;
            case Filtering:
                return AxisRangeInteraction::Filtering;
            case None:
            default:
                return AxisRangeInteraction::None;
        }
    }();
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
        me->setUsed(true);
    } else if (dragRect_ && (me->state() == MouseState::Move)) {
        // convert position from screen coords to data range
        (*dragRect_)[1] = dvec2{me->pos().x, me->pos().y};
        const auto dstart = screenToData_((*dragRect_)[0], me->canvasSize());
        const auto dend = screenToData_((*dragRect_)[1], me->canvasSize());
        const auto mode =
            append ? AxisRangeInteractionMode::Append : AxisRangeInteractionMode::Replace;
        eventCallback_.invoke(AxisRangeEventState::Updated, interaction, mode,
                              std::array<dvec2, 2>{glm::min(dstart, dend), glm::max(dstart, dend)});
        me->setUsed(true);
    }
}

auto BoxSelectionInteractionHandler::addEventCallback(std::function<SelectionFunc> callback)
    -> SelectionCallbackHandle {
    return eventCallback_.add(std::move(callback));
}

void BoxSelectionInteractionHandler::reset() { dragRect_.reset(); }

}  // namespace inviwo::plot
