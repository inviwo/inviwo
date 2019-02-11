/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwo/qt/editor/processordraghelper.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorutils.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/network/autolinker.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/network/networklock.h>

#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/textlabeloverlay.h>

#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/qt/editor/processorportgraphicsitem.h>
#include <inviwo/qt/editor/linkgraphicsitem.h>

#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/qt/editor/processormimedata.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsItem>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QGraphicsView>
#include <warn/pop>

namespace inviwo {

namespace {

template <typename Iter, typename Pred>
std::pair<Iter, Iter> filterAndSortOutports(Iter begin, Iter end, dvec2 pos, Pred pred) {

    end = std::partition(begin, end, [&](Outport* p) {
        return static_cast<double>(util::getPosition(p->getProcessor()).y) +
                       ProcessorGraphicsItem::size_.height() <
                   pos.y &&
               pred(p);
    });

    std::sort(begin, end, [&](Outport* p1, Outport* p2) {
        // weight horizontal distance 3 times over vertical distance
        auto d1 =
            glm::length2(dvec2(3.0, 1.0) * (pos - dvec2(util::getPosition(p1->getProcessor()))));
        auto d2 =
            glm::length2(dvec2(3.0, 1.0) * (pos - dvec2(util::getPosition(p2->getProcessor()))));
        return d1 < d2;
    });
    return {begin, end};
}

}  // namespace

ProcessorDragHelper::ProcessorDragHelper(NetworkEditor& editor)
    : QObject(&editor), editor_{editor} {}

ProcessorDragHelper::~ProcessorDragHelper() = default;

bool ProcessorDragHelper::eventFilter(QObject*, QEvent* event) {
    if (event->type() == QEvent::GraphicsSceneDragEnter) {
        auto e = static_cast<QGraphicsSceneDragDropEvent*>(event);
        if (auto mime = ProcessorMimeData::toProcessorMimeData(e->mimeData())) {
            return enter(e, mime);
        }
    } else if (event->type() == QEvent::GraphicsSceneDragMove) {
        auto e = static_cast<QGraphicsSceneDragDropEvent*>(event);
        if (auto mime = ProcessorMimeData::toProcessorMimeData(e->mimeData())) {
            return move(e, mime);
        }
    } else if (event->type() == QEvent::GraphicsSceneDragLeave) {
        auto e = static_cast<QGraphicsSceneDragDropEvent*>(event);
        if (auto mime = ProcessorMimeData::toProcessorMimeData(e->mimeData())) {
            return leave(e, mime);
        }
    } else if (event->type() == QEvent::GraphicsSceneDrop) {
        auto e = static_cast<QGraphicsSceneDragDropEvent*>(event);
        // for some reason, QGraphicsView accepts the event before it arrives in this event filter
        // @see QGraphicsView::dropEvent(QDropEvent*)
        e->setAccepted(false);
        if (auto mime = ProcessorMimeData::toProcessorMimeData(e->mimeData())) {
            return drop(e, mime);
        }
    }
    event->ignore();
    return false;
}

bool ProcessorDragHelper::enter(QGraphicsSceneDragDropEvent* e, const ProcessorMimeData* mime) {
    e->acceptProposedAction();

    auto processor = mime->processor();
    setText(e, processor->getDisplayName());

    autoConnectCandidates_.clear();
    for (auto& inport : processor->getInports()) {
        std::vector<Outport*> candidates;
        editor_.getNetwork()->forEachProcessor([&](Processor* sourceProcessor) {
            for (auto outport : sourceProcessor->getOutports()) {
                if (inport->canConnectTo(outport)) {
                    candidates.push_back(outport);
                }
            }
        });

        autoConnectCandidates_.emplace_back(inport, std::move(candidates));
    }

    autoLinker_ = std::make_unique<AutoLinker>(editor_.getNetwork(), processor, nullptr);

    move(e, mime);
    return true;
}
bool ProcessorDragHelper::move(QGraphicsSceneDragDropEvent* e, const ProcessorMimeData* mime) {
    e->accept();
    auto processor = mime->processor();
    setText(e, processor->getDisplayName());

    util::setPosition(processor, utilqt::toGLM(e->scenePos()));

    auto connectionItem = editor_.getConnectionGraphicsItemAt(e->scenePos());
    if (connectionItem) {
        if (!connectionTarget_) {
            connectionItem->setBorderColor(canSplitConnection(processor, connectionItem) ? Qt::green
                                                                                         : Qt::red);
            // force update of connection item since color has changed
            connectionItem->update();
            connectionTarget_ = connectionItem;
        }
    } else {
        resetConnection();
    }

    auto processorItem = editor_.getProcessorGraphicsItemAt(e->scenePos());
    if (processorItem) {
        if (!processorTarget_) {
            processorItem->setHighlight(true);
            processorItem->setSelected(true);
            processorTarget_ = processorItem;
        }
    } else {
        resetProcessor();
    }

    if (!processorItem && !connectionItem) {
        updateAutoConnections(e);
    } else {
        resetAutoConnections();
    }

    updateAutoLinks(e);

    return true;
}

void ProcessorDragHelper::updateAutoConnections(QGraphicsSceneDragDropEvent* e) {
    if (!e->modifiers().testFlag(Qt::ShiftModifier)) {
        resetAutoConnections();
        return;
    }
    const auto pos = e->scenePos();
    const auto zoom = 1.0 / editor_.views().front()->transform().m11();

    decltype(autoConnections_) updatedConnections;
    for (auto&& item : util::enumerate(autoConnectCandidates_)) {
        const auto ind = item.first();
        const auto inport = item.second().first;
        auto& outportCandidates = item.second().second;

        auto validOutports = filterAndSortOutports(
            outportCandidates.begin(), outportCandidates.end(), utilqt::toGLM(pos),
            [&](Outport* p) {
                return std::none_of(
                    updatedConnections.begin(), updatedConnections.end(), [&](auto& item) {
                        if (!item.second) return false;
                        return item.second->getOutportGraphicsItem()->getPort() == p;
                    });
            });

        if (validOutports.first != validOutports.second) {
            const auto endpos = pos + zoom * ProcessorGraphicsItem::portOffset(
                                                 ProcessorGraphicsItem::PortType::In, ind);
            auto oldit = autoConnections_.find(inport);
            if (oldit == autoConnections_.end() ||
                oldit->second->getOutportGraphicsItem()->getPort() != *validOutports.first) {
                auto outport = *validOutports.first;

                auto pgi = editor_.getProcessorGraphicsItem(outport->getProcessor());
                auto ogi = pgi->getOutportGraphicsItem(outport);
                auto connection = std::make_unique<ConnectionDragGraphicsItem>(
                    ogi, endpos, utilqt::toQColor(outport->getColorCode()));
                editor_.addItem(connection.get());
                connection->setEndPoint(endpos);
                connection->show();
                connection->update();
                updatedConnections[inport] = std::move(connection);
            } else {
                oldit->second->setEndPoint(endpos);
                oldit->second->update();
                updatedConnections[inport] = std::move(oldit->second);
            }
        }
    }
    std::swap(autoConnections_, updatedConnections);
}

void ProcessorDragHelper::updateAutoLinks(QGraphicsSceneDragDropEvent* e) {
    if (e->modifiers().testFlag(Qt::AltModifier)) {
        resetAutoLinks();
        return;
    }

    const auto pos = e->scenePos();
    const auto zoom = 1.0 / editor_.views().front()->transform().m11();
    decltype(autoLinks_) links;

    autoLinker_->sortAutoLinkCandidates();
    for (auto& item : autoLinker_->getAutoLinkCandidates()) {
        const auto& sourceProperties = item.second;
        if (!sourceProperties.empty()) {
            auto sourceProperty = sourceProperties.front();
            auto sourceProcessor = sourceProperty->getOwner()->getProcessor();

            auto it = autoLinks_.find(sourceProcessor);
            if (it != autoLinks_.end()) {
                if (it->second) {
                    it->second->setEndPoint(pos + zoom * QPointF{-75.0, 0.0},
                                            pos + zoom * QPointF{75.0, 0.0});
                    links[sourceProcessor] = std::move(it->second);
                }
            } else {
                auto pgi = editor_.getProcessorGraphicsItem(sourceProcessor);
                auto lgi = std::make_unique<LinkConnectionDragGraphicsItem>(
                    pgi->getLinkGraphicsItem(), pos);
                editor_.addItem(lgi.get());
                lgi->show();
                links[sourceProcessor] = std::move(lgi);
            }
        }
    }
    std::swap(links, autoLinks_);
}

bool ProcessorDragHelper::leave(QGraphicsSceneDragDropEvent* e, const ProcessorMimeData*) {
    e->accept();
    editor_.getOverlay().clear();
    resetConnection();
    resetProcessor();
    resetAutoConnections();
    resetAutoLinks();
    return true;
}
bool ProcessorDragHelper::drop(QGraphicsSceneDragDropEvent* e, const ProcessorMimeData* mime) {
    e->accept();
    editor_.getOverlay().clear();

    const bool enableAutoLinks = !e->modifiers().testFlag(Qt::AltModifier);
    const bool enableAutoConnections = e->modifiers().testFlag(Qt::ShiftModifier);

    auto network = editor_.getNetwork();
    NetworkLock lock(network);

    util::OnScopeExit resetter{[&]() {
        resetConnection();
        resetProcessor();
        resetAutoConnections();
        resetAutoLinks();
    }};

    try {
        auto processor = mime->get();
        if (!processor) {
            LogError("Unable to get processor from drag object");
            return true;
        }
        editor_.clearSelection();

        util::setPosition(processor.get(), utilqt::toGLM(NetworkEditor::snapToGrid(e->scenePos())));
        if (connectionTarget_) {
            if (!util::addProcessorOnConnection(network, std::move(processor),
                                                connectionTarget_->getPortConnection())) {
                LogError("Unable to add processor on connection");
                return true;
            }
            if (enableAutoLinks) autoLinker_->addLinksToClosestCandidates(true);
        } else if (processorTarget_) {
            util::replaceProcessor(network, std::move(processor), processorTarget_->getProcessor());
        } else if (enableAutoConnections) {
            network->addProcessor(std::move(processor));
            for (auto& item : autoConnections_) {
                if (item.second) {
                    item.second->hide();
                    editor_.getNetwork()->addConnection(
                        item.second->getOutportGraphicsItem()->getPort(), item.first);
                }
            }
            if (enableAutoLinks) autoLinker_->addLinksToClosestCandidates(true);
        } else {
            network->addProcessor(std::move(processor));
            if (enableAutoLinks) autoLinker_->addLinksToClosestCandidates(true);
        }
    } catch (Exception& exception) {
        util::log(exception.getContext(),
                  "Unable to create processor " + utilqt::fromQString(mime->text()) + " due to " +
                      exception.getMessage(),
                  LogLevel::Error);
    }

    return true;
}

void ProcessorDragHelper::resetConnection() {
    if (connectionTarget_) {
        connectionTarget_->resetBorderColors();
        connectionTarget_ = nullptr;
    }
}
void ProcessorDragHelper::resetProcessor() {
    if (processorTarget_) {
        processorTarget_->setHighlight(false);
        processorTarget_->setSelected(false);
        processorTarget_ = nullptr;
    }
}

void ProcessorDragHelper::setText(QGraphicsSceneDragDropEvent* e, const std::string& processor) {
    const auto enableConnections = e->modifiers().testFlag(Qt::ShiftModifier);
    const auto disableAutoLinks = e->modifiers().testFlag(Qt::AltModifier);

    const auto text =
        std::string("Add ") + processor + ". " +
        (enableConnections ? "Auto connections enabled" : "Hold shift to enable auto connect") +
        ". " + (disableAutoLinks ? "Auto Linking disabled" : "Hold Alt to disable auto links") +
        ".";

    editor_.getOverlay().setText(text, std::chrono::milliseconds{3000});
}

void ProcessorDragHelper::resetAutoLinks() { autoLinks_.clear(); }

void ProcessorDragHelper::resetAutoConnections() { autoConnections_.clear(); }

void ProcessorDragHelper::clear(ConnectionGraphicsItem* connection) {
    if (connectionTarget_ == connection) resetConnection();
}
void ProcessorDragHelper::clear(ProcessorGraphicsItem* processor) {
    if (processorTarget_ == processor) resetProcessor();
}

bool ProcessorDragHelper::canSplitConnection(Processor* p, ConnectionGraphicsItem* connection) {
    bool inputmatch = util::any_of(p->getInports(), [&connection](Inport* inport) {
        return inport->canConnectTo(connection->getOutport());
    });
    bool outputmatch = util::any_of(p->getOutports(), [&connection](Outport* outport) {
        return connection->getInport()->canConnectTo(outport);
    });

    return inputmatch && outputmatch;
}

}  // namespace inviwo
