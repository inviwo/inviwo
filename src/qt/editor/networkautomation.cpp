/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <inviwo/qt/editor/networkautomation.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorutils.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/glm.h>

#include <inviwo/qt/editor/networkeditor.h>

#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/qt/editor/processorportgraphicsitem.h>

#include <modules/qtwidgets/textlabeloverlay.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <ranges>

#include <fmt/format.h>

#include <QGraphicsView>

namespace inviwo {

namespace {

void setEditorText(NetworkEditor& editor, Qt::KeyboardModifiers modifiers, bool autoLink) {
    const auto enableInConnections = modifiers.testFlag(NetworkAutomation::autoInport);
    const auto enableOutConnections = modifiers.testFlag(NetworkAutomation::autoOutport);
    const auto disableAutoLinks = modifiers.testFlag(NetworkAutomation::noAutoLink);

    const auto text = fmt::format(
        "{}.\n{}.\n{}",
        (enableInConnections ? "Automatic inport connections enabled"
                             : "Hold Shift to enable automatic inport connections"),
        (enableOutConnections ? "Automatic outport connections enabled"
                              : "Hold Ctrl to enable automatic outport connections"),
        (autoLink ? (disableAutoLinks ? "Auto Linking disabled" : "Hold Alt to disable auto links")
                  : ""));

    editor.getOverlay().setText(text, std::chrono::milliseconds{3000});
}

void clearEditorText(NetworkEditor& editor) { editor.getOverlay().clear(); }

void updateConnectionTarget(ConnectionGraphicsItem*& target, ConnectionGraphicsItem* source,
                            Processor* processor) {
    if (target == source) return;

    if (target) {
        target->resetBorderColors();
        target = nullptr;
    }

    if (source && processor) {
        target = source;
        const auto split = util::canSplitConnection(*processor, target->getPortConnection());
        target->setBorderColor(split ? Qt::green : Qt::red);
        target->update();  // force update of connection item since color has changed
    }
}

void updateProcessorTarget(ProcessorGraphicsItem*& target, ProcessorGraphicsItem* source,
                           Processor* processor) {
    if (target == source) return;

    if (target) {
        target->setHighlight(false);
        target->setSelected(false);
        target = nullptr;
    }

    if (source && source->getProcessor() != processor) {
        target = source;
        target->setHighlight(true);
        target->setSelected(true);
    }
}

template <typename Iter, typename Pred>
std::pair<Iter, Iter> selectAndSortPorts(Iter begin, Iter end, dvec2 pos, Pred pred) {

    auto selectPorts =
        util::overloaded{[](Outport* p, dvec2 pos) {
                             return static_cast<double>(util::getPosition(p->getProcessor()).y) +
                                        ProcessorGraphicsItem::size_.height() <
                                    pos.y;
                         },
                         [](Inport* p, dvec2 pos) {
                             return static_cast<double>(util::getPosition(p->getProcessor()).y) -
                                        ProcessorGraphicsItem::size_.height() >
                                    pos.y;
                         }};

    end = std::partition(begin, end, [&](auto* p) { return selectPorts(p, pos) && pred(p); });

    // weight horizontal distance 3 times over vertical distance
    constexpr auto weighting = dvec2{3.0, 1.0};
    std::stable_sort(begin, end, [&](Port* p1, Port* p2) {
        const auto pos1 = dvec2(util::getPosition(p1->getProcessor()));
        const auto pos2 = dvec2(util::getPosition(p2->getProcessor()));
        const auto d1 = glm::length2(weighting * (pos - pos1));
        const auto d2 = glm::length2(weighting * (pos - pos2));
        return d1 < d2;
    });
    return {begin, end};
}

}  // namespace

void NetworkAutomation::AutoIn::update(QPointF scenePos, Qt::KeyboardModifiers modifiers,
                                       NetworkEditor& editor, double zoom) {
    if (!modifiers.testFlag(NetworkAutomation::autoInport)) {
        connections.clear();
        return;
    }

    const auto inportPos = [&](size_t index) {
        return scenePos +
               zoom * ProcessorGraphicsItem::portOffset(ProcessorGraphicsItem::PortType::In, index);
    };

    std::unordered_map<Inport*, std::unique_ptr<ConnectionOutportDragGraphicsItem>>
        updatedConnections;
    const auto isPortFree = [&updatedConnections](Outport* p) {
        return std::none_of(updatedConnections.begin(), updatedConnections.end(), [&](auto& item) {
            if (!item.second) return false;
            return item.second->getOutportGraphicsItem()->getPort() == p;
        });
    };

    for (auto&& [index, pair] : util::enumerate(candidates)) {
        auto&& [inport, outportCandidates] = pair;

        auto validOutports = selectAndSortPorts(outportCandidates.begin(), outportCandidates.end(),
                                                utilqt::toGLM(scenePos), isPortFree);

        if (validOutports.first != validOutports.second) {
            const auto endPos = inportPos(index);
            auto oldIt = connections.find(inport);
            if (oldIt == connections.end() ||
                oldIt->second->getOutportGraphicsItem()->getPort() != *validOutports.first) {

                auto* outport = *(validOutports.first);
                auto* pgi = editor.getProcessorGraphicsItem(outport->getProcessor());
                auto* ogi = pgi->getOutportGraphicsItem(outport);
                auto connection = std::make_unique<ConnectionOutportDragGraphicsItem>(
                    ogi, endPos, utilqt::toQColor(outport->getColorCode()));
                editor.addItem(connection.get());
                connection->setEndPoint(endPos);
                connection->show();
                connection->update();
                updatedConnections[inport] = std::move(connection);
            } else {
                oldIt->second->setEndPoint(endPos);
                oldIt->second->update();
                updatedConnections[inport] = std::move(oldIt->second);
            }
        }
    }
    std::swap(connections, updatedConnections);
}

void NetworkAutomation::AutoOut::update(QPointF scenePos, Qt::KeyboardModifiers modifiers,
                                        NetworkEditor& editor, double zoom) {

    if (!modifiers.testFlag(NetworkAutomation::autoOutport)) {
        connections.clear();
        return;
    }

    const auto outportPos = [&](size_t index) {
        return scenePos + zoom * ProcessorGraphicsItem::portOffset(
                                     ProcessorGraphicsItem::PortType::Out, index);
    };

    std::unordered_map<Outport*, std::unique_ptr<ConnectionInportDragGraphicsItem>>
        updatedConnections;
    const auto isPortFree = [&updatedConnections](Inport* p) {
        return std::none_of(updatedConnections.begin(), updatedConnections.end(), [&](auto& item) {
            if (!item.second) return false;
            return item.second->getInportGraphicsItem()->getPort() == p;
        });
    };

    for (auto&& [index, pair] : util::enumerate(candidates)) {
        auto&& [outport, inportCandidates] = pair;

        auto validInports = selectAndSortPorts(inportCandidates.begin(), inportCandidates.end(),
                                               utilqt::toGLM(scenePos), isPortFree);

        if (validInports.first != validInports.second) {
            const auto startPos = outportPos(index);
            auto oldIt = connections.find(outport);
            if (oldIt == connections.end() ||
                oldIt->second->getInportGraphicsItem()->getPort() != *validInports.first) {

                auto* inport = *(validInports.first);
                auto* pgi = editor.getProcessorGraphicsItem(inport->getProcessor());
                auto* ogi = pgi->getInportGraphicsItem(inport);
                auto connection = std::make_unique<ConnectionInportDragGraphicsItem>(
                    startPos, ogi, utilqt::toQColor(outport->getColorCode()));
                editor.addItem(connection.get());
                connection->setStartPoint(startPos);
                connection->show();
                connection->update();
                updatedConnections[outport] = std::move(connection);
            } else {
                oldIt->second->setStartPoint(startPos);
                oldIt->second->update();
                updatedConnections[outport] = std::move(oldIt->second);
            }
        }
    }
    std::swap(connections, updatedConnections);
}

void NetworkAutomation::AutoIn::findCandidates(Processor& processor, ProcessorNetwork& network) {
    candidates.clear();
    for (auto* inport : processor.getInports()) {

        std::vector<Outport*> targets;
        if (!inport->isConnected()) {
            network.forEachProcessor([&](Processor* sourceProcessor) {
                for (auto* outport : sourceProcessor->getOutports()) {
                    if (inport->canConnectTo(outport)) {
                        targets.push_back(outport);
                    }
                }
            });
        }
        candidates.emplace_back(inport, std::move(targets));
    }
}

void NetworkAutomation::AutoIn::clear() {
    connections.clear();
    candidates.clear();
}

void NetworkAutomation::AutoOut::findCandidates(Processor& processor, ProcessorNetwork& network) {
    candidates.clear();
    for (auto* outport : processor.getOutports()) {
        std::vector<Inport*> targets;
        if (!outport->isConnected()) {
            network.forEachProcessor([&](Processor* sourceProcessor) {
                for (auto* inport : sourceProcessor->getInports()) {
                    if (inport->canConnectTo(outport)) {
                        targets.push_back(inport);
                    }
                }
            });
        }
        candidates.emplace_back(outport, std::move(targets));
    }
}
void NetworkAutomation::AutoOut::clear() {
    connections.clear();
    candidates.clear();
}

void NetworkAutomation::AutoLink::findCandidates(Processor& processor, ProcessorNetwork& network) {
    linker.update(network, processor);
}

void NetworkAutomation::AutoLink::update(QPointF scenePos, Qt::KeyboardModifiers modifiers,
                                         NetworkEditor& editor, double zoom) {

    if (modifiers.testFlag(NetworkAutomation::noAutoLink)) {
        links.clear();
        return;
    }

    decltype(links) updatedLinks;

    linker.sortAutoLinkCandidates();
    for (const auto& item : linker.getAutoLinkCandidates()) {
        const auto& sourceProperties = item.second;
        if (!sourceProperties.empty()) {
            auto* sourceProperty = sourceProperties.front();
            auto* sourceProcessor = sourceProperty->getOwner()->getProcessor();

            auto it = links.find(sourceProcessor);
            if (it != links.end()) {
                if (it->second) {
                    it->second->setEndPoint(scenePos + zoom * QPointF{-75.0, 0.0},
                                            scenePos + zoom * QPointF{75.0, 0.0});
                    updatedLinks[sourceProcessor] = std::move(it->second);
                }
            } else {
                auto* pgi = editor.getProcessorGraphicsItem(sourceProcessor);
                auto lgi = std::make_unique<LinkConnectionDragGraphicsItem>(
                    pgi->getLinkGraphicsItem(), scenePos);
                editor.addItem(lgi.get());
                lgi->show();
                updatedLinks[sourceProcessor] = std::move(lgi);
            }
        }
    }
    std::swap(links, updatedLinks);
}

void NetworkAutomation::AutoLink::clear() {
    links.clear();
    linker.clear();
}

NetworkAutomation::NetworkAutomation(NetworkEditor& editor)
    : editor_{editor}, inports_{}, outports_{}, links_{} {}

void NetworkAutomation::enter(QPointF scenePos, Qt::KeyboardModifiers modifiers,
                              Processor& processor, double zoom) {

    autoLink_ =
        std::ranges::empty(editor_.getNetwork()->processorRange() |
                           std::views::filter([&](const auto& p) { return &p == &processor; }));

    inports_.findCandidates(processor, *(editor_.getNetwork()));
    outports_.findCandidates(processor, *(editor_.getNetwork()));
    if (autoLink_) {
        links_.findCandidates(processor, *(editor_.getNetwork()));
    }

    move(scenePos, modifiers, processor, zoom);
}
void NetworkAutomation::move(QPointF scenePos, Qt::KeyboardModifiers modifiers,
                             Processor& processor, double zoom) {
    setEditorText(editor_, modifiers, autoLink_);

    updateConnectionTarget(connectionTarget_, editor_.getConnectionGraphicsItemAt(scenePos),
                           &processor);
    updateProcessorTarget(processorTarget_, editor_.getProcessorGraphicsItemAt(scenePos),
                          &processor);

    if (!processorTarget_ && !connectionTarget_) {
        inports_.update(scenePos, modifiers, editor_, zoom);
        outports_.update(scenePos, modifiers, editor_, zoom);
    } else {
        inports_.connections.clear();
        outports_.connections.clear();
    }

    if (autoLink_) {
        links_.update(scenePos, modifiers, editor_, zoom);
    }
}
void NetworkAutomation::leave() {

    clearEditorText(editor_);

    updateConnectionTarget(connectionTarget_, nullptr, nullptr);
    updateProcessorTarget(processorTarget_, nullptr, nullptr);

    inports_.clear();
    outports_.clear();
    links_.clear();
}
auto NetworkAutomation::drop(QPointF, Qt::KeyboardModifiers modifiers, Processor& processor)
    -> Result {

    const util::OnScopeExit resetter{[&]() { leave(); }};

    auto result = Result::None;
    if (connectionTarget_ &&
        util::canSplitConnection(processor, connectionTarget_->getPortConnection())) {
        if (util::addProcessorOnConnection(*editor_.getNetwork(), processor,
                                           connectionTarget_->getPortConnection())) {
            connectionTarget_ = nullptr;
            result = Result::Split;

        } else {
            log::error("Unable to add processor on connection");
        }
    } else if (processorTarget_) {
        util::replaceProcessor(editor_.getNetwork(), processor, processorTarget_->getProcessor());
        processorTarget_ = nullptr;  // the target processor has been removed
        result = Result::Replace;
    } else {
        if (modifiers.testFlag(autoInport)) {
            for (auto& [inport, cgi] : inports_.connections) {
                if (cgi) {
                    cgi->hide();
                    editor_.getNetwork()->addConnection(cgi->getOutportGraphicsItem()->getPort(),
                                                        inport);
                }
            }
        }
        if (modifiers.testFlag(autoOutport)) {
            for (auto& [outport, cgi] : outports_.connections) {
                if (cgi) {
                    cgi->hide();

                    auto* inport = cgi->getInportGraphicsItem()->getPort();
                    if (inport->getNumberOfConnections() >= inport->getMaxNumberOfConnections()) {
                        editor_.getNetwork()->removeConnection(inport->getConnectedOutport(),
                                                               inport);
                    }

                    editor_.getNetwork()->addConnection(outport, inport);
                }
            }
        }
        result = Result::AutoConnect;
    }

    if (autoLink_ && result != Result::Replace &&
        !modifiers.testFlag(NetworkAutomation::noAutoLink)) {
        links_.linker.addLinksToClosestCandidates(*(editor_.getNetwork()), true);
    }

    return result;
}

void NetworkAutomation::clear(ConnectionGraphicsItem* connection) {
    if (connectionTarget_ == connection) {
        updateConnectionTarget(connectionTarget_, nullptr, nullptr);
    }
}
void NetworkAutomation::clear(ProcessorGraphicsItem* processor) {
    if (processorTarget_ == processor) {
        updateProcessorTarget(processorTarget_, nullptr, nullptr);
    }
}

}  // namespace inviwo
