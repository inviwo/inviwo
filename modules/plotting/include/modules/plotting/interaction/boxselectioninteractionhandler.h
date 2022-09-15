/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2022 Inviwo Foundation
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

#include <modules/plotting/plottingmoduledefine.h>  // for IVW_MODULE_PLOTTING_API

#include <inviwo/core/interaction/interactionhandler.h>  // for InteractionHandler
#include <inviwo/core/util/dispatcher.h>                 // for Dispatcher
#include <inviwo/core/util/glmvec.h>                     // for dvec2, size2_t

#include <array>       // for array
#include <functional>  // for function
#include <memory>      // for shared_ptr
#include <optional>    // for optional
#include <string>      // for string
#include <vector>      // for vector

namespace inviwo {
class BufferBase;
class Event;

namespace plot {
class BoxSelectionProperty;

/**
 * \brief Handles interaction for 2D rectangle selection/filtering
 * Selection/Filtering callbacks are called when filtering changes.
 *
 * The current drag rectangle is given by getDragRectangle.
 * @see DragRectangleRenderer
 */
class IVW_MODULE_PLOTTING_API BoxSelectionInteractionHandler : public InteractionHandler {
public:
    /**
     * \brief Selection/filtering-changed callback.
     * The index of each element is given by its location (0, 1 ... n).
     * Second argument specifies if the selection/filtering should be appended.
     */
    using SelectionFunc = void(const std::vector<bool>&, bool);
    using SelectionCallbackHandle = std::shared_ptr<std::function<SelectionFunc>>;
    /**
     * \brief Handles interaction for 2D rectangle selection/filtering
     * Selection/Filtering callbacks are called when filtering changes.
     * @param boxSelectionSettings use for selection/filtering/none
     * @param xAxis data
     * @param yAxis data
     * @param screenToData converts screen coordinates to (x,y) data coordinates
     */
    BoxSelectionInteractionHandler(const BoxSelectionProperty& boxSelectionSettings,
                                   std::shared_ptr<const BufferBase> xAxis,
                                   std::shared_ptr<const BufferBase> yAxis,
                                   std::function<dvec2(dvec2 p, const size2_t& dims)> screenToData);
    virtual ~BoxSelectionInteractionHandler() = default;

    virtual void invokeEvent(Event* event) override;

    virtual std::string getClassIdentifier() const override;
    /**
     * \brief Added callbacks will be called when selection changed.
     */
    SelectionCallbackHandle addSelectionChangedCallback(std::function<SelectionFunc> callback);
    /**
     * \brief Added callbacks will be called when filtering changed.
     */
    SelectionCallbackHandle addFilteringChangedCallback(std::function<SelectionFunc> callback);

    void setXAxisData(std::shared_ptr<const BufferBase> buffer);
    void setYAxisData(std::shared_ptr<const BufferBase> buffer);
    /**
     * \brief Returns (lower, upper) screen space coordinates of selection rectangle, null if not
     * active.
     */
    std::optional<std::array<dvec2, 2>> getDragRectangle() const { return dragRect_; }
    /**
     * \brief Reset the drag rectangle
     * @see getDragRectangle
     */
    void reset();

protected:
    /**
     * \brief React to rectangle drag changes. Input is in data-space of each axis.
     */
    void dragRectChanged(const dvec2& start, const dvec2& end, bool append);
    std::vector<bool> boxSelect(const dvec2& start, const dvec2& end, const BufferBase* xAxis_,
                                const BufferBase* yAxis_);
    std::vector<bool> boxFilter(const dvec2& start, const dvec2& end, const BufferBase* xAxis_,
                                const BufferBase* yAxis_);

    Dispatcher<SelectionFunc> selectionChangedCallback_;
    Dispatcher<SelectionFunc> filteringChangedCallback_;
    const BoxSelectionProperty& dragRectSettings_;  ///! Selection/filtering
    std::shared_ptr<const BufferBase> xAxis_;
    std::shared_ptr<const BufferBase> yAxis_;

    std::function<dvec2(dvec2 p, const size2_t& dims)> screenToData_;
    std::optional<std::array<dvec2, 2>> dragRect_;
};

}  // namespace plot

}  // namespace inviwo
