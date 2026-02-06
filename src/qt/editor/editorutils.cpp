/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/qt/editor/editorutils.h>

#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/qt/editor/networkeditor.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/processors/processorutils.h>

namespace inviwo::utilqt {

ivec2 findSpaceForProcessors(QPoint srcPos, const std::vector<Processor*>& added,
                             const std::vector<Processor*>& current) {
    constexpr auto pSize = ProcessorGraphicsItem::size.toSize();
    constexpr QRect processorRect{QPoint{-pSize.width() / 2, -pSize.height() / 2}, pSize};

    constexpr auto grid = QSize{NetworkEditor::gridSpacing, NetworkEditor::gridSpacing};
    constexpr auto delta = pSize + grid;

    const auto bounds = util::getBoundingBox(added);
    const auto size = utilqt::toQSize(bounds.second - bounds.first) + pSize;
    const QRect addedRect{
        QPoint{-pSize.width() / 2 - grid.width(), -pSize.height() / 2 - grid.height()},
        size + 2 * grid};
    const auto positions = util::getPositions(current);

    auto targetPos =
        QPoint{std::numeric_limits<int>::max() / 2, std::numeric_limits<int>::max() / 2};

    const auto distance = [&](QPoint targetPos) -> float {
        return glm::length(
            vec2{utilqt::toGLM(targetPos) + utilqt::toGLM(size) / 2 - utilqt::toGLM(srcPos)});
    };

    const auto overlapping = [&](QPoint addedPos) {
        const auto bb = addedRect.translated(addedPos);
        return std::ranges::any_of(positions, [&](auto p) {
            return bb.intersects(processorRect.translated(utilqt::toQPoint(p)));
        });
    };

    const auto searchLeft = [&](QPoint startPos) -> QPoint {
        while (overlapping(startPos) && distance(startPos) < distance(targetPos)) {
            startPos.rx() -= NetworkEditor::gridSpacing;
        }
        return startPos;
    };
    const auto searchRight = [&](QPoint startPos) -> QPoint {
        while (overlapping(startPos) && distance(startPos) < distance(targetPos)) {
            startPos.rx() += NetworkEditor::gridSpacing;
        }
        return startPos;
    };

    for (auto startPos = QPoint{srcPos.x(), srcPos.y() + delta.height()};
         distance(startPos) < distance(targetPos); startPos.ry() += NetworkEditor::gridSpacing) {
        const auto left = searchLeft(startPos);
        const auto right = searchRight(startPos);
        if (distance(left) < distance(targetPos)) targetPos = left;
        if (distance(right) < distance(targetPos)) targetPos = right;
    }
    const auto offset = utilqt::toGLM(targetPos) - bounds.first;

    return offset;
}

void addProcessorAndConnect(std::shared_ptr<Processor> processor, ProcessorNetwork* net,
                            Outport* outport) {

    const auto srcPos = utilqt::toQPoint(util::getPosition(outport->getProcessor()));
    const auto current = net->getProcessors();
    const auto offset = utilqt::findSpaceForProcessors(srcPos, {processor.get()}, current);
    util::offsetPosition(processor.get(), offset);
    net->addProcessor(processor);
    for (auto* inport : processor->getInports()) {

        if (inport->canConnectTo(outport)) {
            net->addConnection(outport, inport);
            break;
        }
    }
}

void addProcessorAndConnect(std::shared_ptr<Processor> processor, ProcessorNetwork* net,
                            Processor* source) {

    const auto srcPos = utilqt::toQPoint(util::getPosition(source));
    const auto current = net->getProcessors();
    const auto offset = utilqt::findSpaceForProcessors(srcPos, {processor.get()}, current);
    util::offsetPosition(processor.get(), offset);
    net->addProcessor(processor);
    for (auto* outport : source->getOutports()) {
        for (auto* inport : processor->getInports()) {
            if (inport->canConnectTo(outport)) {
                net->addConnection(outport, inport);
                return;
            }
        }
    }
}

}  // namespace inviwo::utilqt
