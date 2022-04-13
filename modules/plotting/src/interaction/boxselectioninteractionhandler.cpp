/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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

#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/util/zip.h>

#include <limits>

namespace inviwo {

namespace plot {

BoxSelectionInteractionHandler::BoxSelectionInteractionHandler(
    const BoxSelectionProperty& dragRectSettings, std::shared_ptr<const BufferBase> xAxis,
    std::shared_ptr<const BufferBase> yAxis,
    std::function<dvec2(dvec2 p, const size2_t& dims)> screenToData)
    : InteractionHandler()
    , dragRectSettings_(dragRectSettings)
    , xAxis_(xAxis)
    , yAxis_(yAxis)
    , screenToData_(screenToData) {}

void BoxSelectionInteractionHandler::invokeEvent(Event* event) {
    if (event->hash() == MouseEvent::chash()) {
        auto me = event->getAs<MouseEvent>();
        auto append = me->modifiers().contains(KeyModifier::Control);
        if ((me->button() == MouseButton::Left) && (me->state() == MouseState::Press)) {
            dragRect_ = {dvec2{me->pos().x, me->pos().y}, dvec2{me->pos().x, me->pos().y}};
            me->setUsed(true);
        } else if ((me->button() == MouseButton::Left) && (me->state() == MouseState::Release)) {
            if (dragRect_ && glm::compMax(glm::abs(me->pos() - (*dragRect_)[0])) <= 1) {
                // Click in empty space
                switch (dragRectSettings_.getMode()) {
                    case BoxSelectionSettingsInterface::Mode::Selection: {
                        // selection changed
                        std::vector<bool> selected(xAxis_->getSize(), false);
                        selectionChangedCallback_.invoke(selected, append);
                        break;
                    }
                    case BoxSelectionSettingsInterface::Mode::Filtering: {
                        std::vector<bool> filtered(xAxis_->getSize(), false);
                        filteringChangedCallback_.invoke(filtered, append);
                        break;
                    }
                    case BoxSelectionSettingsInterface::Mode::None:
                        break;
                }
            }
            dragRect_ = std::nullopt;
            me->setUsed(true);
        } else if ((me->buttonState() & MouseButton::Left) && (me->state() == MouseState::Move)) {
            // convert position from screen coords to data range
            (*dragRect_)[1] = dvec2{me->pos().x, me->pos().y};
            auto dstart = screenToData_((*dragRect_)[0], me->canvasSize());
            auto dend = screenToData_((*dragRect_)[1], me->canvasSize());
            dragRectChanged(glm::min(dstart, dend), glm::max(dstart, dend), append);
            me->setUsed(true);
        }
    }
}

std::string BoxSelectionInteractionHandler::getClassIdentifier() const {
    return "org.inviwo.BoxSelectInteractionHandler";
}

auto BoxSelectionInteractionHandler::addSelectionChangedCallback(
    std::function<SelectionFunc> callback) -> SelectionCallbackHandle {
    return selectionChangedCallback_.add(callback);
}

auto BoxSelectionInteractionHandler::addFilteringChangedCallback(
    std::function<SelectionFunc> callback) -> SelectionCallbackHandle {
    return filteringChangedCallback_.add(callback);
}

void BoxSelectionInteractionHandler::setXAxisData(std::shared_ptr<const BufferBase> buffer) {
    xAxis_ = buffer;
}

void BoxSelectionInteractionHandler::setYAxisData(std::shared_ptr<const BufferBase> buffer) {
    yAxis_ = buffer;
}

void BoxSelectionInteractionHandler::reset() { dragRect_.reset(); }

void BoxSelectionInteractionHandler::dragRectChanged(const dvec2& start, const dvec2& end,
                                                     bool append) {
    switch (dragRectSettings_.getMode()) {
        case BoxSelectionSettingsInterface::Mode::Selection:
            // selection changed
            selectionChangedCallback_.invoke(boxSelect(start, end, xAxis_.get(), yAxis_.get()),
                                             append);
            break;
        case BoxSelectionSettingsInterface::Mode::Filtering:
            filteringChangedCallback_.invoke(boxFilter(start, end, xAxis_.get(), yAxis_.get()),
                                             append);
            break;
        case BoxSelectionSettingsInterface::Mode::None:
            break;
    }
}

std::vector<bool> BoxSelectionInteractionHandler::boxSelect(const dvec2& start, const dvec2& end,
                                                            const BufferBase* xAxis,
                                                            const BufferBase* yAxis) {

    if (xAxis == nullptr || yAxis == nullptr) {
        return std::vector<bool>();
    }

    // For efficiency:
    // 1. Determine selection along x-axis
    // 2. Determine selection along y-axis using the subset from 1

    auto xbuf = xAxis->getRepresentation<BufferRAM>();
#include <warn/push>
#include <warn/ignore/conversion>  // Ignore double->float warnings
    auto selectedIndicesX = xbuf->dispatch<std::vector<bool>, dispatching::filter::Scalars>(
        [min = start[0], max = end[0]](auto brprecision) {
            using ValueType = util::PrecisionValueType<decltype(brprecision)>;
            std::vector<bool> selected(brprecision->getSize(), false);
            // Avoid conversions in the loop

            const auto tmin = std::numeric_limits<ValueType>::is_integer
                                  ? static_cast<ValueType>(std::ceil(min))
                                  : static_cast<ValueType>(min);
            const auto tmax = std::numeric_limits<ValueType>::is_integer
                                  ? static_cast<ValueType>(std::floor(max))
                                  : static_cast<ValueType>(max);
            for (auto&& [ind, elem] : util::enumerate(brprecision->getDataContainer())) {
                if (elem < tmin || elem > tmax) {
                    continue;
                } else {
                    selected[ind] = true;
                }
            }
            return selected;
        });
    // Use indices filted by x-axis as input
    auto ybuf = yAxis->getRepresentation<BufferRAM>();
    auto selectedIndices = ybuf->dispatch<std::vector<bool>, dispatching::filter::Scalars>(
        [selectedIndicesX, min = start[1], max = end[1]](auto brprecision) {
            using ValueType = util::PrecisionValueType<decltype(brprecision)>;
            auto data = brprecision->getDataContainer();
            std::vector<bool> selected(brprecision->getSize(), false);
            // Avoid conversions in the loop
            const auto tmin = std::numeric_limits<ValueType>::is_integer
                                  ? static_cast<ValueType>(std::ceil(min))
                                  : static_cast<ValueType>(min);
            const auto tmax = std::numeric_limits<ValueType>::is_integer
                                  ? static_cast<ValueType>(std::floor(max))
                                  : static_cast<ValueType>(max);

            for (auto&& [ind, elem] : util::enumerate(selectedIndicesX)) {
                if (elem) {
                    if (data[ind] < tmin || data[ind] > tmax) {
                        continue;
                    } else {
                        selected[ind] = true;
                    }
                }
            }
            return selected;
        });
#include <warn/pop>
    return selectedIndices;
}

std::vector<bool> BoxSelectionInteractionHandler::boxFilter(const dvec2& start, const dvec2& end,
                                                            const BufferBase* xAxis,
                                                            const BufferBase* yAxis) {
    if (xAxis == nullptr || yAxis == nullptr) {
        return std::vector<bool>();
    }
    auto xbuf = xAxis->getRepresentation<BufferRAM>();
#include <warn/push>
#include <warn/ignore/conversion>  // Ignore double->float warnings
    auto filteredIndices = xbuf->dispatch<std::vector<bool>, dispatching::filter::Scalars>(
        [start, end, ybuf = yAxis->getRepresentation<BufferRAM>()](auto brprecision) {
            using ValueTypeX = util::PrecisionValueType<decltype(brprecision)>;
            // Avoid conversions in the loop
            const auto tminX = std::numeric_limits<ValueTypeX>::is_integer
                                   ? static_cast<ValueTypeX>(std::ceil(start[0]))
                                   : static_cast<ValueTypeX>(start[0]);
            const auto tmaxX = std::numeric_limits<ValueTypeX>::is_integer
                                   ? static_cast<ValueTypeX>(std::floor(end[0]))
                                   : static_cast<ValueTypeX>(end[0]);
            auto xData = brprecision->getDataContainer();
            return ybuf->dispatch<std::vector<bool>, dispatching::filter::Scalars>(
                [tminX, tmaxX, start, end, xData](auto brprecision) {
                    using ValueTypeY = util::PrecisionValueType<decltype(brprecision)>;
                    std::vector<bool> filtered(brprecision->getSize(), false);
                    const auto tminY = std::numeric_limits<ValueTypeY>::is_integer
                                           ? static_cast<ValueTypeY>(std::ceil(start[1]))
                                           : static_cast<ValueTypeY>(start[1]);
                    const auto tmaxY = std::numeric_limits<ValueTypeY>::is_integer
                                           ? static_cast<ValueTypeY>(std::floor(end[1]))
                                           : static_cast<ValueTypeY>(end[1]);

                    for (auto&& [ind, xVal, yVal] :
                         util::enumerate(xData, brprecision->getDataContainer())) {
                        if ((xVal < tminX) || (xVal > tmaxX) || (yVal < tminY) || (yVal > tmaxY)) {
                            filtered[ind] = true;
                        } else {
                            continue;
                        }
                    }
                    return filtered;
                });
        });

#include <warn/pop>

    return filteredIndices;
}

}  // namespace plot

}  // namespace inviwo
