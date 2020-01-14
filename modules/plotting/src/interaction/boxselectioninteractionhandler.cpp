/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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
                    case BoxSelectionSettingsInterface::Mode::Selection:
                        // selection changed
                        selectionChangedCallback_.invoke(std::unordered_set<size_t>{}, append);
                        break;
                    case BoxSelectionSettingsInterface::Mode::Filtering:
                        filteringChangedCallback_.invoke(std::unordered_set<size_t>{}, append);
                        break;
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

std::unordered_set<size_t> BoxSelectionInteractionHandler::boxSelect(const dvec2& start,
                                                                     const dvec2& end,
                                                                     const BufferBase* xAxis,
                                                                     const BufferBase* yAxis) {
    if (xAxis == nullptr || yAxis == nullptr) {
        return std::unordered_set<size_t>();
    }
    // For efficiency:
    // 1. Determine selection along x-axis
    // 2. Determine selection along y-axis using the subset from 1

    auto xbuf = xAxis->getRepresentation<BufferRAM>();
    auto selectedIndicesX = xbuf->dispatch<std::vector<size_t>, dispatching::filter::Scalars>(
        [min = start[0], max = end[0]](auto brprecision) {
            std::vector<size_t> selectedIndices;
            selectedIndices.reserve(brprecision->getSize());
            for (auto&& [ind, elem] : util::enumerate(brprecision->getDataContainer())) {
                if (static_cast<double>(elem) < min || static_cast<double>(elem) > max) {
                    continue;
                } else {
                    selectedIndices.emplace_back(ind);
                }
            }
            return selectedIndices;
        });
    // Use indices filted by x-axis as input
    auto ybuf = yAxis->getRepresentation<BufferRAM>();
    auto selectedIndices = ybuf->dispatch<std::unordered_set<size_t>, dispatching::filter::Scalars>(
        [selectedIndicesX, min = start[1], max = end[1]](auto brprecision) {
            using ValueType = util::PrecisionValueType<decltype(brprecision)>;
            auto data = brprecision->getDataContainer();
            std::unordered_set<size_t> selectedIndices;
            selectedIndices.reserve(selectedIndicesX.size());

            std::copy_if(selectedIndicesX.begin(), selectedIndicesX.end(),
                         std::inserter(selectedIndices, selectedIndices.begin()),
                         [data, min, max](auto idx) {
                             return !(static_cast<double>(data[idx]) < min ||
                                      static_cast<double>(data[idx]) > max);
                         });

            return selectedIndices;
        });
    return selectedIndices;
}

std::unordered_set<size_t> BoxSelectionInteractionHandler::boxFilter(const dvec2& start,
                                                                     const dvec2& end,
                                                                     const BufferBase* xAxis,
                                                                     const BufferBase* yAxis) {
    if (xAxis == nullptr || yAxis == nullptr) {
        return std::unordered_set<size_t>();
    }
    auto xbuf = xAxis->getRepresentation<BufferRAM>();
    std::unordered_set<size_t> filteredIndices =
        xbuf->dispatch<std::unordered_set<size_t>, dispatching::filter::Scalars>(
            [start, end, ybuf = yAxis->getRepresentation<BufferRAM>()](auto brprecision) {
                using ValueType = util::PrecisionValueType<decltype(brprecision)>;
                return ybuf->dispatch<std::unordered_set<size_t>, dispatching::filter::Scalars>(
                    [start, end, xData = brprecision->getDataContainer()](auto brprecision) {
                        std::unordered_set<size_t> selectedIndices;
                        for (auto&& [ind, xVal, yVal] :
                             util::enumerate(xData, brprecision->getDataContainer())) {
                            if (static_cast<double>(xVal) < start[0] ||
                                static_cast<double>(xVal) > end[0] ||
                                static_cast<double>(yVal) < start[1] ||
                                static_cast<double>(yVal) > end[1]) {
                                selectedIndices.insert(ind);
                            } else {
                                continue;
                            }
                        }
                        return selectedIndices;
                    });
            });

    return filteredIndices;
}

}  // namespace plot

}  // namespace inviwo
