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
#pragma once

#include <modules/plotting/plottingmoduledefine.h>

#include <inviwo/core/interaction/interactionhandler.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/util/dispatcher.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/interaction/axisrangeeventstate.h>

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace inviwo {
class BufferBase;
class Event;
class MouseEvent;

namespace plot {
class BoxSelectionProperty;

/**
 * @brief Handles interaction for 2D rectangle selection/filtering
 * Selection/Filtering callbacks are called when filtering changes.
 *
 * The current drag rectangle is given by getDragRectangle.
 *
 * The event properties returned by properties() need to be added to a processor in order for the
 * interactions to work.
 *
 * @see DragRectangleRenderer
 */
class IVW_MODULE_PLOTTING_API BoxSelectionInteractionHandler : public InteractionHandler {
public:
    using SelectionFunc = void(AxisRangeEventState, AxisRangeInteraction, AxisRangeInteractionMode,
                               std::optional<std::array<dvec2, 2>>);
    using SelectionCallbackHandle = std::shared_ptr<std::function<SelectionFunc>>;

    /**
     * @brief Handles interaction for 2D rectangle selection/filtering
     * The callbacks are called when filtering changes.
     * @param boxSelectionSettings use for selection/filtering/none
     * @param screenToData converts screen coordinates to (x,y) data coordinates
     */
    BoxSelectionInteractionHandler(const BoxSelectionProperty& boxSelectionSettings,
                                   std::function<dvec2(dvec2 p, const size2_t& dims)> screenToData);
    virtual ~BoxSelectionInteractionHandler() = default;

    /**
     * @brief Added callbacks will be called when a box selection is initiated, updated, and
     * finished
     */
    SelectionCallbackHandle addEventCallback(std::function<SelectionFunc> callback);

    /**
     * @brief Returns (lower, upper) screen space coordinates of selection rectangle, null if not
     * active.
     */
    std::optional<std::array<dvec2, 2>> getDragRectangle() const { return dragRect_; }
    /**
     * @brief Reset the drag rectangle
     * @see getDragRectangle
     */
    void reset();

    auto properties() { return std::tie(mouseBoxSelection_, mouseBoxSelectionAppend_); }
    auto properties() const { return std::tie(mouseBoxSelection_, mouseBoxSelectionAppend_); }

protected:
    void mouseEvent(MouseEvent* event, bool append);

    Dispatcher<SelectionFunc> eventCallback_;
    const BoxSelectionProperty& dragRectSettings_;  ///! Selection/filtering
    std::function<dvec2(dvec2 p, const size2_t& dims)> screenToData_;
    std::optional<std::array<dvec2, 2>> dragRect_;

    EventProperty mouseBoxSelection_;
    EventProperty mouseBoxSelectionAppend_;
};

}  // namespace plot

}  // namespace inviwo
