/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#include <inviwo/qt/editor/networkeditor.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/io/serialization/deserializer.h>
#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/metadata/processorwidgetmetadata.h>
#include <inviwo/core/network/processornetworkevaluator.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/processors/compositeprocessorutils.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/ports/portinspectormanager.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/processors/compositeprocessor.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/processors/processorwidgetfactory.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/processors/processorutils.h>
#include <inviwo/core/util/factory.h>
#include <inviwo/core/util/settings/linksettings.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/editor/linkdialog/linkdialog.h>
#include <inviwo/qt/editor/linkgraphicsitem.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/qt/editor/processorlinkgraphicsitem.h>
#include <inviwo/qt/editor/processorlistwidget.h>
#include <inviwo/qt/editor/processorportgraphicsitem.h>
#include <inviwo/qt/editor/processorprogressgraphicsitem.h>
#include <inviwo/qt/editor/processorerroritem.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/qt/editor/helpwidget.h>
#include <inviwo/qt/editor/processorstatusgraphicsitem.h>
#include <inviwo/qt/editor/processormimedata.h>
#include <modules/qtwidgets/eventconverterqt.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/propertylistwidget.h>

#include <inviwo/core/rendering/datavisualizermanager.h>

#include <inviwo/qt/editor/processordraghelper.h>
#include <inviwo/qt/editor/connectiondraghelper.h>
#include <inviwo/qt/editor/linkdraghelper.h>
#include <inviwo/qt/editor/subpropertyselectiondialog.h>

#include <QApplication>
#include <QClipboard>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QVarLengthArray>
#include <QTimer>
#include <QPainter>
#include <QMimeData>
#include <QMargins>
#include <QGraphicsView>
#include <QGraphicsSimpleTextItem>
#include <QInputDialog>

#include <fmt/std.h>

namespace inviwo {

const int NetworkEditor::gridSpacing_ = 25;

NetworkEditor::NetworkEditor(InviwoMainWindow* mainWindow)
    : QGraphicsScene()
    , processorDragHelper_{new ProcessorDragHelper(*this)}
    , linkDragHelper_{new LinkDragHelper(*this)}
    , connectionOutDragHelper_{new ConnectionOutDragHelper(*this)}
    , connectionInDragHelper_{new ConnectionInDragHelper(*this)}
    , processorItem_{nullptr}
    , automation_{*this}
    , mainWindow_(mainWindow)
    , network_(mainWindow->getInviwoApplication()->getProcessorNetwork())
    , backgroundVisible_(true)
    , adjustSceneToChange_(true) {

    setObjectName(name);

    mainWindow->getInviwoApplication()->getProcessorNetworkEvaluator()->setExceptionHandler(
        [this](Processor* processor, EvaluationType type, ExceptionContext context) {
            const auto& id = processor->getIdentifier();
            const auto error = [&](std::string error) {
                if (auto pgi = getProcessorGraphicsItem(processor)) {
                    pgi->getStatusItem()->setRuntimeError();
                    pgi->setErrorText(error);
                }
            };

            try {
                throw;
            } catch (const Exception& e) {
                error(fmt::format("{}: {}", type, e.getMessage()));
                util::log(e.getContext(),
                          fmt::format("{} Error in {} : {}", id, type, e.getFullMessage()),
                          LogLevel::Error);
            } catch (const fmt::format_error& e) {
                error(fmt::format("{}: {}", type, e.what()));
                util::log(context,
                          fmt::format("{} Error in {} using fmt formatting: {}\n{}", id, type,
                                      e.what(), util::fmtHelp.view()),
                          LogLevel::Error);
            } catch (const std::exception& e) {
                error(fmt::format("{}: {}", type, e.what()));
                util::log(context, fmt::format("{} Error in {} : {}", id, type, e.what()),
                          LogLevel::Error);
            } catch (...) {
                error(fmt::format("{}: Unknown error", type));
                util::log(context, fmt::format("{} Error in {} : Unknown error", id, type),
                          LogLevel::Error);
            }
        });

    network_->addObserver(this);

    installEventFilter(processorDragHelper_);

    setSceneRect(QRectF());

    // If selection contains processors don't select any other items
    connect(this, &QGraphicsScene::selectionChanged, this, [this]() {
        auto selection = selectedItems();
        if (util::contains_if(selection, [](auto i) {
                return qgraphicsitem_cast<ProcessorGraphicsItem*>(i) != nullptr;
            })) {
            for (auto i : selection) {
                if (qgraphicsitem_cast<ProcessorGraphicsItem*>(i) == nullptr) {
                    i->setSelected(false);
                }
            }
        }
    });
}

ProcessorNetwork* NetworkEditor::getNetwork() const { return network_; }
InviwoMainWindow* NetworkEditor::getMainWindow() const { return mainWindow_; }
TextLabelOverlay& NetworkEditor::getOverlay() const {
    return mainWindow_->getNetworkEditorOverlay();
}

ProcessorGraphicsItem* NetworkEditor::addProcessorRepresentations(Processor* processor) {
    // generate GUI representations (graphics item, property widget, processor widget)
    ProcessorGraphicsItem* ret = addProcessorGraphicsItem(processor);

    if (auto processorWidget = processor->getProcessorWidget()) {
        if (auto md = processor->getMetaData<BoolMetaData>("PortInspector")) {
            if (md->get()) {
                if (auto widget = dynamic_cast<QWidget*>(processorWidget)) {
                    auto size = processorWidget->getDimensions();
                    widget->setMinimumSize(size.x, size.y);
                    widget->setMaximumSize(size.x, size.y);
                    widget->setWindowFlags(Qt::CustomizeWindowHint | Qt::Tool);
                }
                processorWidget->setVisible(true);
            }
        }
    }

    return ret;
}

void NetworkEditor::removeProcessorRepresentations(Processor* processor) {
    removeProcessorGraphicsItem(processor);
    removeAndDeletePropertyWidgets(processor);
}

ProcessorGraphicsItem* NetworkEditor::addProcessorGraphicsItem(Processor* processor) {
    // generate GUI representation and add to editor
    auto processorGraphicsItem = new ProcessorGraphicsItem(processor);
    processorGraphicsItems_[processor] = processorGraphicsItem;
    addItem(processorGraphicsItem);
    updateSceneSize();
    if (adjustSceneToChange_ && processorGraphicsItem->isVisible()) {
        for (auto v : views()) {
            v->ensureVisible(processorGraphicsItem);
        }
    }
    return processorGraphicsItem;
}

void NetworkEditor::removeProcessorGraphicsItem(Processor* processor) {
    // obtain processor graphics item through processor
    auto processorGraphicsItem = getProcessorGraphicsItem(processor);
    if (processorItem_ == processorGraphicsItem) {
        processorItem_ = nullptr;
    }
    automation_.clear(processorGraphicsItem);
    processorDragHelper_->clear(processorGraphicsItem);

    removeItem(processorGraphicsItem);
    processorGraphicsItems_.erase(processor);
    // delete processor graphics item
    delete processorGraphicsItem;

    updateSceneSize();
}

void NetworkEditor::addPropertyWidgets(Processor* processor) {
    auto it = std::find_if(
        processorGraphicsItems_.begin(), processorGraphicsItems_.end(),
        [&](const auto& item) { return item.first != processor && item.second->isSelected(); });
    if (it == processorGraphicsItems_.end() ||
        QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
        QCoreApplication::postEvent(
            mainWindow_->getPropertyListWidget(),
            new PropertyListEvent(PropertyListEvent::Action::Add, processor->getIdentifier()),
            Qt::LowEventPriority);
    }
}

void NetworkEditor::removePropertyWidgets(Processor* processor) {
    QCoreApplication::postEvent(
        mainWindow_->getPropertyListWidget(),
        new PropertyListEvent(PropertyListEvent::Action::Remove, processor->getIdentifier()),
        Qt::LowEventPriority);
}

void NetworkEditor::removeAndDeletePropertyWidgets(Processor* processor) {
    // Will not use events here since we might delete the processor
    mainWindow_->getPropertyListWidget()->removeAndDeleteProcessorProperties(processor);
}

ConnectionGraphicsItem* NetworkEditor::addConnectionGraphicsItem(const PortConnection& connection) {
    Outport* outport = connection.getOutport();
    Inport* inport = connection.getInport();

    auto outProcessor = getProcessorGraphicsItem(outport->getProcessor());
    auto inProcessor = getProcessorGraphicsItem(inport->getProcessor());

    auto connectionGraphicsItem =
        new ConnectionGraphicsItem(outProcessor->getOutportGraphicsItem(outport),
                                   inProcessor->getInportGraphicsItem(inport), connection);

    connectionGraphicsItems_[connection] = connectionGraphicsItem;
    addItem(connectionGraphicsItem);
    connectionGraphicsItem->setVisible(outProcessor->isVisible() && inProcessor->isVisible());

    return connectionGraphicsItem;
}

void NetworkEditor::removeConnection(ConnectionGraphicsItem* connectionGraphicsItem) {
    Outport* outport = connectionGraphicsItem->getOutport();
    Inport* inport = connectionGraphicsItem->getInport();

    automation_.clear(connectionGraphicsItem);
    processorDragHelper_->clear(connectionGraphicsItem);

    RenderContext::getPtr()->activateDefaultRenderContext();
    network_->removeConnection(outport, inport);
}

void NetworkEditor::removeConnectionGraphicsItem(const PortConnection& connection) {
    ConnectionGraphicsItem* connectionGraphicsItem = connectionGraphicsItems_[connection];
    processorDragHelper_->clear(connectionGraphicsItem);
    connectionGraphicsItems_.erase(connection);
    delete connectionGraphicsItem;
}

LinkConnectionGraphicsItem* NetworkEditor::addLinkGraphicsItem(Processor* processor1,
                                                               Processor* processor2) {
    if (processor1 && processor2) {
        auto p1 = getProcessorGraphicsItem(processor1)->getLinkGraphicsItem();
        auto p2 = getProcessorGraphicsItem(processor2)->getLinkGraphicsItem();

        auto linkGraphicsItem = new LinkConnectionGraphicsItem(p1, p2);
        linkGraphicsItems_[ProcessorPair(processor1, processor2)] = linkGraphicsItem;
        addItem(linkGraphicsItem);
        return linkGraphicsItem;
    }
    return nullptr;
}

void NetworkEditor::removeLink(LinkConnectionGraphicsItem* linkGraphicsItem) {
    auto links = network_->getLinksBetweenProcessors(
        linkGraphicsItem->getSrcProcessorGraphicsItem()->getProcessor(),
        linkGraphicsItem->getDestProcessorGraphicsItem()->getProcessor());

    RenderContext::getPtr()->activateDefaultRenderContext();
    NetworkLock lock(network_);
    for (auto& link : links) {
        network_->removeLink(link.getSource(), link.getDestination());
    }
}

void NetworkEditor::removeLinkGraphicsItem(LinkConnectionGraphicsItem* linkGraphicsItem) {
    if (linkGraphicsItem) {
        linkGraphicsItems_.erase(
            ProcessorPair(linkGraphicsItem->getDestProcessorGraphicsItem()->getProcessor(),
                          linkGraphicsItem->getSrcProcessorGraphicsItem()->getProcessor()));
        delete linkGraphicsItem;
    }
}

void NetworkEditor::showLinkDialog(Processor* processor1, Processor* processor2) {
    auto dialog = new LinkDialog(processor1, processor2, mainWindow_);
    dialog->show();
}

std::shared_ptr<const Image> NetworkEditor::renderPortInspectorImage(Outport* outport) {
    auto pim = mainWindow_->getInviwoApplication()->getPortInspectorManager();
    return pim->renderPortInspectorImage(outport);
}

ProcessorGraphicsItem* NetworkEditor::getProcessorGraphicsItem(Processor* key) const {
    return util::map_find_or_null(processorGraphicsItems_, key);
}

ConnectionGraphicsItem* NetworkEditor::getConnectionGraphicsItem(const PortConnection& key) const {
    return util::map_find_or_null(connectionGraphicsItems_, key);
}

LinkConnectionGraphicsItem* NetworkEditor::getLinkGraphicsItem(const ProcessorPair& key) const {
    return util::map_find_or_null(linkGraphicsItems_, key);
}

LinkConnectionGraphicsItem* NetworkEditor::getLinkGraphicsItem(Processor* processor1,
                                                               Processor* processor2) const {
    return getLinkGraphicsItem(ProcessorPair(processor1, processor2));
}

void NetworkEditor::setBackgroundVisible(bool visible) {
    if (visible == backgroundVisible_) {
        return;
    }
    backgroundVisible_ = visible;
    invalidate(QRectF(), SceneLayer::BackgroundLayer);
}

bool NetworkEditor::isBackgroundVisible() const { return backgroundVisible_; }

void NetworkEditor::updateSceneSize() {
    if (adjustSceneToChange_ && !processorItem_) {
        setSceneRect(getProcessorsBoundingRect());
        forEachObserver([&](auto o) { o->onSceneSizeChanged(); });
    }
}

QRectF NetworkEditor::getProcessorsBoundingRect() const {
    QRectF rect;
    for (const auto& item : processorGraphicsItems_) {
        if (item.second->isVisible()) {
            rect = rect.united(item.second->sceneBoundingRect());
        }
    }
    return rect;
}

std::string NetworkEditor::getMimeTag() { return "application/x.vnd.inviwo.network+xml"; }

ProcessorGraphicsItem* NetworkEditor::getProcessorGraphicsItemAt(const QPointF pos) const {
    return getGraphicsItemAt<ProcessorGraphicsItem>(pos);
}
ProcessorInportGraphicsItem* NetworkEditor::getProcessorInportGraphicsItemAt(
    const QPointF pos) const {
    return getGraphicsItemAt<ProcessorInportGraphicsItem>(pos);
}

ProcessorOutportGraphicsItem* NetworkEditor::getProcessorOutportGraphicsItemAt(
    const QPointF pos) const {
    return getGraphicsItemAt<ProcessorOutportGraphicsItem>(pos);
}

ConnectionGraphicsItem* NetworkEditor::getConnectionGraphicsItemAt(const QPointF pos) const {
    return getGraphicsItemAt<ConnectionGraphicsItem>(pos);
}
LinkConnectionGraphicsItem* NetworkEditor::getLinkGraphicsItemAt(const QPointF pos) const {
    return getGraphicsItemAt<LinkConnectionGraphicsItem>(pos);
}

void NetworkEditor::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    if (auto p = getProcessorGraphicsItemAt(e->scenePos())) {
        processorItem_ = p;
        automation_.enter(processorItem_->pos(), e->modifiers(), *processorItem_->getProcessor());
    }
    QGraphicsScene::mousePressEvent(e);
}

void NetworkEditor::mouseMoveEvent(QGraphicsSceneMouseEvent* e) {
    if (processorItem_) {
        automation_.move(processorItem_->pos(), e->modifiers(), *processorItem_->getProcessor());
    }

    QGraphicsScene::mouseMoveEvent(e);
}

void NetworkEditor::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
    for (auto& elem : processorGraphicsItems_) {
        const QPointF pos = elem.second->pos();
        const QPointF newPos = snapToGrid(pos);
        if (pos != newPos) elem.second->setPos(newPos);
    }

    if (processorItem_) {
        automation_.drop(processorItem_->pos(), e->modifiers(), *processorItem_->getProcessor());
        processorItem_ = nullptr;
        updateSceneSize();
    } else {
        automation_.leave();
    }

    QGraphicsScene::mouseReleaseEvent(e);
}

void NetworkEditor::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) {
    if (auto p = getProcessorGraphicsItemAt(e->scenePos())) {
        if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
            for (auto item : selectedItems()) {
                if (auto gp = qgraphicsitem_cast<ProcessorGraphicsItem*>(item)) {
                    for (auto proc : util::getDirectSuccessors(gp->getProcessor())) {
                        if (auto it = processorGraphicsItems_.find(proc);
                            it != processorGraphicsItems_.end()) {
                            it->second->setSelected(true);
                        }
                    }
                    for (auto proc : util::getDirectPredecessors(gp->getProcessor())) {
                        if (auto it = processorGraphicsItems_.find(proc);
                            it != processorGraphicsItems_.end()) {
                            it->second->setSelected(true);
                        }
                    }
                }
            }
        } else {
            if (auto processor = p->getProcessor()) {
                if (auto widget = processor->getProcessorWidget()) {
                    widget->setVisible(!widget->isVisible());
                }
            }
        }
        e->accept();

    } else if (auto linkGraphicsItem = getLinkGraphicsItemAt(e->scenePos())) {
        showLinkDialog(linkGraphicsItem->getSrcProcessorGraphicsItem()->getProcessor(),
                       linkGraphicsItem->getDestProcessorGraphicsItem()->getProcessor());
        e->accept();

    } else {
        QGraphicsScene::mouseDoubleClickEvent(e);
    }
}

void NetworkEditor::keyPressEvent(QKeyEvent* keyEvent) {
    QGraphicsScene::keyPressEvent(keyEvent);

    if (processorItem_) {
        automation_.move(processorItem_->pos(), keyEvent->modifiers(),
                         *processorItem_->getProcessor());
    }

    if (!keyEvent->isAccepted()) {
        KeyboardEvent pressKeyEvent(utilqt::getKeyButton(keyEvent), KeyState::Press,
                                    utilqt::getModifiers(keyEvent), keyEvent->nativeVirtualKey(),
                                    utilqt::fromQString(keyEvent->text()));
        propagateEventToSelectedProcessors(pressKeyEvent);
    }
}

void NetworkEditor::keyReleaseEvent(QKeyEvent* keyEvent) {
    QGraphicsScene::keyPressEvent(keyEvent);

    if (processorItem_) {
        automation_.move(processorItem_->pos(), keyEvent->modifiers(),
                         *processorItem_->getProcessor());
    }

    if (!keyEvent->isAccepted()) {
        KeyboardEvent releaseKeyEvent(utilqt::getKeyButton(keyEvent), KeyState::Release,
                                      utilqt::getModifiers(keyEvent), keyEvent->nativeVirtualKey(),
                                      utilqt::fromQString(keyEvent->text()));
        propagateEventToSelectedProcessors(releaseKeyEvent);
    }
}

void NetworkEditor::deleteAndKeepConnections(ProcessorGraphicsItem* processor) {
    RenderContext::getPtr()->activateDefaultRenderContext();

    auto p = processor->getProcessor();
    for (auto& prop : p->getPropertiesRecursive()) {
        auto links = network_->getPropertiesLinkedTo(prop);
        auto bidirectional = util::copy_if(
            links, [&](Property* dst) { return network_->isLinkedBidirectional(prop, dst); });
        if (bidirectional.size() > 1) {
            for (size_t i = 1; i < bidirectional.size(); ++i) {
                network_->addLink(bidirectional[0], bidirectional[i]);
                network_->addLink(bidirectional[i], bidirectional[0]);
            }
        }
    }

    auto& inports = p->getInports();
    auto& outports = p->getOutports();
    for (size_t i = 0; i < std::min(inports.size(), outports.size()); ++i) {
        if (inports[i]->isConnected() && outports[i]->isConnected()) {
            auto out = inports[i]->getConnectedOutport();
            auto ins = outports[i]->getConnectedInports();
            network_->removeConnection(out, inports[i]);
            for (auto in : ins) {
                network_->removeConnection(outports[i], in);
                network_->addConnection(out, in);
            }
        }
    }
    network_->removeProcessor(p);
}

void NetworkEditor::contextMenuEvent(QGraphicsSceneContextMenuEvent* e) {

    auto addVisualizers = [this](QMenu& menu, ProcessorOutportGraphicsItem* ogi) {
        auto outport = ogi->getPort();

        auto app = mainWindow_->getInviwoApplication();
        auto pim = app->getPortInspectorManager();

        if (pim->isPortInspectorSupported(outport)) {
            auto pos = ogi->mapPosToSceen(ogi->rect().center());
            auto hasInspector = pim->hasPortInspector(outport);
            auto showPortInsector =
                menu.addAction(tr(hasInspector ? "Hide Port Inspector" : "Show Port &Inspector"));
            showPortInsector->setCheckable(true);
            showPortInsector->setChecked(hasInspector);
            connect(showPortInsector, &QAction::triggered, [pim, outport, pos]() {
                if (!pim->hasPortInspector(outport)) {
                    pim->addPortInspector(outport, ivec2(pos.x(), pos.y()));
                } else {
                    pim->removePortInspector(outport);
                }
            });
        }

        auto dataVis = app->getDataVisualizerManager()->getDataVisualizersForOutport(outport);
        if (!dataVis.empty()) {
            auto subMenu = menu.addMenu("Add Visualizer");
            for (auto vis : dataVis) {
                auto action = subMenu->addAction(utilqt::toQString(vis->getName()));
                connect(action, &QAction::triggered, [this, vis, outport]() {
                    AdjustSceneToChangesBlocker blocker(*this);
                    RenderContext::getPtr()->activateDefaultRenderContext();

                    auto pos = util::getPosition(outport->getProcessor());
                    auto oldPos = util::getPositions(network_);
                    auto added = vis->addVisualizerNetwork(outport, network_);
                    // add below pos
                    const auto bounds = util::getBoundingBox(added);
                    for (auto p : oldPos) {
                        if ((p.x > pos.x - ProcessorGraphicsItem::size_.width() / 2) &&
                            (p.x <
                             pos.x + bounds.second.x + ProcessorGraphicsItem::size_.width() / 2)) {
                            pos.y = std::max(p.y, pos.y);
                        }
                    }

                    const auto offset = pos + ivec2{0, 25 + ProcessorGraphicsItem::size_.height()} -
                                        ivec2{bounds.first.x, bounds.first.y};
                    util::offsetPosition(added, offset);
                });
            }
        }

        if (auto imagePort = dynamic_cast<ImageOutport*>(outport)) {
            if (auto image = imagePort->getData()) {
                menu.addSeparator();
                utilqt::addImageActions(menu, *image);
            }
        }
    };

    QMenu menu;
    ProcessorGraphicsItem* clickedProcessor = nullptr;

    for (auto& item : items(e->scenePos())) {
        if (auto outport = qgraphicsitem_cast<ProcessorOutportGraphicsItem*>(item)) {
            addVisualizers(menu, outport);
            break;

        } else if (auto inport = qgraphicsitem_cast<ProcessorInportGraphicsItem*>(item)) {
            if (inport->getPort()->isConnected()) {
                auto port = inport->getConnections().front()->getOutportGraphicsItem();
                addVisualizers(menu, port);
            }
            break;

        } else if (auto connection = qgraphicsitem_cast<ConnectionGraphicsItem*>(item)) {
            auto port = connection->getOutportGraphicsItem();
            addVisualizers(menu, port);
            break;

        } else if (auto processor = qgraphicsitem_cast<ProcessorGraphicsItem*>(item)) {
            clickedProcessor = processor;

            if (auto widget = processor->getProcessor()->getProcessorWidget()) {
                QAction* showAction = menu.addAction(tr("&Show Widget"));
                showAction->setCheckable(true);
                showAction->setChecked(widget->isVisible());
                connect(showAction, &QAction::triggered,
                        [widget]() { widget->setVisible(!widget->isVisible()); });
            } else if (auto comp = dynamic_cast<CompositeProcessor*>(processor->getProcessor())) {
                if (util::any_of(comp->getSubNetwork().processorRange(),
                                 [](const Processor& p) { return p.hasProcessorWidget(); })) {
                    QMenu* subMenu = menu.addMenu("Widgets");
                    for (const auto& subProcessor : comp->getSubNetwork().processorRange()) {
                        if (auto subWidget = subProcessor.getProcessorWidget()) {
                            QAction* showAction = subMenu->addAction(utilqt::toQString(
                                fmt::format("Show {} Widget", subProcessor.getDisplayName())));
                            showAction->setCheckable(true);
                            showAction->setChecked(subWidget->isVisible());
                            connect(showAction, &QAction::triggered, [subWidget]() {
                                subWidget->setVisible(!subWidget->isVisible());
                            });
                        }
                    }
                }
            }

            auto editName = menu.addAction(tr("Edit Name"));
            connect(editName, &QAction::triggered, [this, processor]() {
                clearSelection();
                processor->setSelected(true);

                bool ok{false};
                QString text = QInputDialog::getText(
                    nullptr, "Rename", "Name:", QLineEdit::Normal,
                    utilqt::toQString(processor->getProcessor()->getDisplayName()), &ok,
                    Qt::WindowFlags() | Qt::MSWindowsFixedSizeDialogHint);
                if (ok && !text.isEmpty()) {
                    try {
                        processor->getProcessor()->setDisplayName(utilqt::fromQString(text));
                    } catch (const Exception& e) {
                        LogError(e.getMessage());
                    }
                }
            });

            auto editIdentifier = menu.addAction(tr("Edit Identifier"));
            connect(editIdentifier, &QAction::triggered, [this, processor]() {
                clearSelection();
                processor->setSelected(true);

                bool ok{false};
                QString text = QInputDialog::getText(
                    nullptr, "Rename", "Identifier:", QLineEdit::Normal,
                    utilqt::toQString(processor->getProcessor()->getIdentifier()), &ok,
                    Qt::WindowFlags() | Qt::MSWindowsFixedSizeDialogHint);
                if (ok && !text.isEmpty()) {
                    try {
                        processor->getProcessor()->setIdentifier(utilqt::fromQString(text));
                    } catch (const Exception& e) {
                        LogError(e.getMessage());
                    }
                }
            });

#if IVW_PROFILING
            QAction* resetTimeMeasurementsAction =
                menu.addAction(QIcon(":svgicons/timer.svg"), tr("Reset &Time Measurements"));
            connect(resetTimeMeasurementsAction, &QAction::triggered,
                    [processor]() { processor->resetTimeMeasurements(); });
#endif

            QAction* delprocessor = menu.addAction(QIcon(":/svgicons/edit-delete.svg"),
                                                   tr("Delete && &Keep Connections"));
            connect(delprocessor, &QAction::triggered,
                    [this, processor]() { deleteAndKeepConnections(processor); });

            menu.addSeparator();
            QAction* invalidateOutputAction = menu.addAction(tr("Invalidate &Output"));
            connect(invalidateOutputAction, &QAction::triggered, [processor]() {
                processor->getProcessor()->invalidate(InvalidationLevel::InvalidOutput);
            });

            QAction* invalidateResourcesAction = menu.addAction(tr("Invalidate &Resources"));
            connect(invalidateResourcesAction, &QAction::triggered, [processor]() {
                processor->getProcessor()->invalidate(InvalidationLevel::InvalidResources);
            });

            menu.addSeparator();

            QAction* showPropAction = menu.addAction(tr("Show &Properties"));
            connect(showPropAction, &QAction::triggered, [this, processor]() {
                auto plw = std::make_unique<PropertyListWidget>(
                    mainWindow_, mainWindow_->getInviwoApplication());
                plw->setFloating(true);
                plw->addProcessorProperties(processor->getProcessor());
                plw->setWindowTitle(utilqt::toQString(processor->getProcessor()->getDisplayName()));
                plw->show();
                processor->adoptWidget(std::move(plw));
            });

            QAction* internalLink = menu.addAction(tr("Show Internal &Links"));
            connect(internalLink, &QAction::triggered,
                    [this, processor = processor->getProcessor()]() {
                        auto dialog = new LinkDialog(processor, processor, mainWindow_);
                        dialog->show();
                    });

            menu.addSeparator();

            QAction* selectPre = menu.addAction(tr("Select Predecessors"));
            connect(selectPre, &QAction::triggered,
                    [this, processor = processor->getProcessor()]() {
                        for (auto p : util::getPredecessors(processor)) {
                            if (auto it = processorGraphicsItems_.find(p);
                                it != processorGraphicsItems_.end()) {
                                it->second->setSelected(true);
                            }
                        }
                    });
            QAction* selectSuc = menu.addAction(tr("Select Successors"));
            connect(selectSuc, &QAction::triggered,
                    [this, processor = processor->getProcessor()]() {
                        for (auto p : util::getSuccessors(processor)) {
                            if (auto it = processorGraphicsItems_.find(p);
                                it != processorGraphicsItems_.end()) {
                                it->second->setSelected(true);
                            }
                        }
                    });

            menu.addSeparator();

            QAction* helpAction = menu.addAction(QIcon(":/svgicons/help.svg"), tr("Show &Help"));
            connect(helpAction, &QAction::triggered, [this, processor]() {
                showProcessorHelp(processor->getProcessor()->getClassIdentifier(), true);
            });

            break;

        } else if (auto link = qgraphicsitem_cast<LinkConnectionGraphicsItem*>(item)) {
            QAction* editLink = menu.addAction(tr("Edit Links"));
            connect(editLink, &QAction::triggered, [this, link]() {
                showLinkDialog(link->getSrcProcessorGraphicsItem()->getProcessor(),
                               link->getDestProcessorGraphicsItem()->getProcessor());
            });
            break;
        }
    }

    std::vector<ProcessorMap::value_type> selectedProcessors;
    util::copy_if(processorGraphicsItems_, std::back_inserter(selectedProcessors),
                  [](ProcessorMap::value_type item) { return item.second->isSelected(); });
    if (selectedProcessors.size() == 2) {
        QAction* editLink = menu.addAction(tr("Make Link"));
        connect(editLink, &QAction::triggered, [this, selectedProcessors]() {
            showLinkDialog(selectedProcessors[0].second->getProcessor(),
                           selectedProcessors[1].second->getProcessor());
        });
    }

    clickedOnItems_.append(items(e->scenePos()));
    clickedPosition_ = {true, utilqt::toGLM(e->scenePos())};
    {
        menu.addSeparator();
        auto compAction = menu.addAction(QIcon(":/svgicons/composite-create-enabled.svg"),
                                         tr("&Create Composite"));
        connect(compAction, &QAction::triggered, this, [this]() {
            RenderContext::getPtr()->activateDefaultRenderContext();
            util::replaceSelectionWithCompositeProcessor(*network_);
        });
        compAction->setEnabled(selectedProcessors.size() > 1);

        auto expandAction = menu.addAction(QIcon(":/svgicons/composite-expand-enabled.svg"),
                                           tr("&Expand Composite"));
        std::unordered_set<CompositeProcessor*> selectedComposites;
        for (auto& p : selectedProcessors) {
            if (auto comp = dynamic_cast<CompositeProcessor*>(p.first)) {
                selectedComposites.insert(comp);
            }
        }
        for (auto item : clickedOnItems_) {
            if (auto pgi = qgraphicsitem_cast<ProcessorGraphicsItem*>(item)) {
                if (auto comp = dynamic_cast<CompositeProcessor*>(pgi->getProcessor())) {
                    selectedComposites.insert(comp);
                }
            }
        }
        connect(expandAction, &QAction::triggered, this, [selectedComposites]() {
            for (auto& p : selectedComposites) {
                util::expandCompositeProcessorIntoNetwork(*p);
            }
        });
        expandAction->setDisabled(selectedComposites.empty());

        auto selectPropAction =
            menu.addAction(QIcon(":/svgicons/developermode.svg"), tr("Configure Properties"));
        selectPropAction->setEnabled(selectedComposites.size() == 1);
        connect(selectPropAction, &QAction::triggered, this, [this, selectedComposites]() {
            auto dialog = new SubPropertySelectionDialog(*selectedComposites.begin(), mainWindow_);
            dialog->show();
        });

        auto saveCompAction = menu.addAction(QIcon(":/svgicons/save.svg"), tr("&Save Composite"));
        connect(saveCompAction, &QAction::triggered, this, [selectedComposites]() {
            for (auto& p : selectedComposites) {
                const auto compDir = filesystem::getPath(PathType::Settings, "/composites", true);
                const auto filename = util::findUniqueIdentifier(
                    util::stripIdentifier(p->getDisplayName()),
                    [&](std::string_view name) {
                        auto path = compDir / fmt::format("{}.inv", name);
                        return !std::filesystem::is_regular_file(path);
                    },
                    "");
                std::filesystem::create_directories(compDir);
                const auto path = compDir / (filename + ".inv");
                p->saveSubNetwork(path);
                LogInfo("Saved Composite to \"" << path
                                                << "\". \nComposite is now available in the "
                                                   "Processor list (restart may be required).");
            }
        });
        saveCompAction->setDisabled(selectedComposites.empty());

        menu.addSeparator();
        auto cutAction = menu.addAction(QIcon(":/svgicons/edit-cut.svg"), tr("Cu&t"));
        cutAction->setEnabled(clickedProcessor || selectedItems().size() > 0);
        connect(cutAction, &QAction::triggered, this, [this]() {
            auto mimeData = cut();
            QApplication::clipboard()->setMimeData(mimeData.release());
        });

        auto copyAction = menu.addAction(QIcon(":/svgicons/edit-copy.svg"), tr("&Copy"));
        copyAction->setEnabled(clickedProcessor || selectedItems().size() > 0);
        connect(copyAction, &QAction::triggered, this, [this]() {
            auto mimeData = copy();
            QApplication::clipboard()->setMimeData(mimeData.release());
        });

        auto pasteAction = menu.addAction(QIcon(":/svgicons/edit-paste.svg"), tr("&Paste"));
        auto mimeData = QApplication::clipboard()->mimeData();
        if (mimeData->formats().contains(utilqt::toQString(getMimeTag()))) {
            pasteAction->setEnabled(true);
        } else if (mimeData->formats().contains(QString("text/plain"))) {
            pasteAction->setEnabled(true);
        } else {
            pasteAction->setEnabled(false);
        }
        connect(pasteAction, &QAction::triggered, this, [this]() {
            if (auto mimeData = QApplication::clipboard()->mimeData()) {
                paste(*mimeData);
            }
        });

        menu.addSeparator();

        auto deleteAction = menu.addAction(QIcon(":/svgicons/edit-delete.svg"), tr("&Delete"));
        deleteAction->setEnabled(clickedOnItems_.size() + selectedItems().size() > 0);
        connect(deleteAction, &QAction::triggered, this, [this]() { deleteSelection(); });
    }

    menu.exec(QCursor::pos());
    e->accept();
    clickedOnItems_.clear();
    clickedPosition_ = {false, ivec2{0, 0}};
}

void NetworkEditor::showProcessorHelp(const std::string& classIdentifier, bool raise /*= false*/) {
    auto help = mainWindow_->getHelpWidget();
    if (raise) {
        if (!help->isVisible()) help->show();
        help->raise();
    }
    help->showDocForClassName(classIdentifier);
}

void NetworkEditor::propagateEventToSelectedProcessors(KeyboardEvent& pressKeyEvent) {
    try {
        for (auto& item : selectedItems()) {
            if (auto pgi = qgraphicsitem_cast<ProcessorGraphicsItem*>(item)) {
                Processor* p = pgi->getProcessor();
                p->propagateEvent(&pressKeyEvent, nullptr);
                if (pressKeyEvent.hasBeenUsed()) break;
            }
        }
    } catch (const Exception& e) {
        util::log(e.getContext(), e.getFullMessage());
    }
}

void NetworkEditor::deleteSelection() {
    deleteItems(clickedOnItems_);
    clickedOnItems_.clear();
    deleteItems(selectedItems());
}

void NetworkEditor::deleteItems(QList<QGraphicsItem*> items) {
    NetworkLock lock(network_);
    RenderContext::getPtr()->activateDefaultRenderContext();

    // Remove Connections
    util::erase_remove_if(items, [&](QGraphicsItem* item) {
        if (auto cgi = qgraphicsitem_cast<ConnectionGraphicsItem*>(item)) {
            removeConnection(cgi);
            return true;
        } else {
            return false;
        }
    });

    // Remove Links
    util::erase_remove_if(items, [&](QGraphicsItem* item) {
        if (auto lgi = qgraphicsitem_cast<LinkConnectionGraphicsItem*>(item)) {
            removeLink(lgi);
            return true;
        } else {
            return false;
        }
    });

    // Remove Processors. It is important to remove processors last.
    util::erase_remove_if(items, [&](QGraphicsItem* item) {
        if (auto pgi = qgraphicsitem_cast<ProcessorGraphicsItem*>(item)) {
            network_->removeProcessor(pgi->getProcessor());
            return true;
        } else {
            return false;
        }
    });
}

std::unique_ptr<QMimeData> NetworkEditor::copy() const {
    auto copyError = [](auto&& container) -> std::unique_ptr<QMimeData> {
        if (container.size() == 1) {
            if (auto error = qgraphicsitem_cast<ProcessorErrorItem*>(container.front())) {
                const auto str = utilqt::fromQString(error->text());
                QByteArray byteArray(str.c_str(), static_cast<int>(str.length()));
                auto mimeData = std::make_unique<QMimeData>();
                mimeData->setData(QString("text/plain"), byteArray);
                return mimeData;
            }
        }
        return nullptr;
    };

    if (auto mimeData = copyError(selectedItems())) {
        return mimeData;
    }
    if (auto mimeData = copyError(clickedOnItems_)) {
        return mimeData;
    }

    std::stringstream ss;
    std::vector<ProcessorGraphicsItem*> items;
    for (auto& item : clickedOnItems_) {
        if (auto processor = qgraphicsitem_cast<ProcessorGraphicsItem*>(item)) {
            if (!processor->isSelected()) {
                processor->setSelected(true);
                items.push_back(processor);
            }
        }
    }

    util::serializeSelected(network_, ss, "");
    auto str = ss.str();
    QByteArray byteArray(str.c_str(), static_cast<int>(str.length()));

    for (auto& item : items) item->setSelected(false);

    pastePos_.first = false;

    auto mimeData = std::make_unique<QMimeData>();
    mimeData->setData(utilqt::toQString(NetworkEditor::getMimeTag()), byteArray);
    mimeData->setData(QString("text/plain"), byteArray);

    return mimeData;
}

std::unique_ptr<QMimeData> NetworkEditor::cut() {
    auto res = copy();
    deleteSelection();
    return res;
}

void NetworkEditor::paste(const QMimeData& mimeData) {
    QByteArray data;
    if (mimeData.formats().contains(utilqt::toQString(NetworkEditor::getMimeTag()))) {
        data = mimeData.data(utilqt::toQString(NetworkEditor::getMimeTag()));
    } else if (mimeData.formats().contains(QString("text/plain"))) {
        data = mimeData.data(QString("text/plain"));
    } else {
        return;
    }

    NetworkLock lock(network_);
    try {
        auto offsetCallback = [&, orgBounds = util::getBoundingBox(network_)](
                                  const std::vector<Processor*>& added) -> ivec2 {
            auto center = util::getCenterPosition(added);
            auto bounds = util::getBoundingBox(added);

            ivec2 offset{0, 0};
            if (clickedPosition_.first) {  // center on clicked pos
                offset = clickedPosition_.second - center;
            } else if (pastePos_.first) {
                pastePos_.second.x += (bounds.second.x - bounds.first.x) +
                                      static_cast<int>(ProcessorGraphicsItem::size_.width()) +
                                      gridSpacing_;
                offset = pastePos_.second - center;
            } else {  // add to bottom left
                pastePos_.first = true;
                pastePos_.second = ivec2{orgBounds.first.x, orgBounds.second.y} +
                                   ivec2{(bounds.second.x - bounds.first.x) / 2,
                                         (bounds.second.y - bounds.first.y) / 2} +
                                   ivec2{0, gridSpacing_} +
                                   ivec2{0, ProcessorGraphicsItem::size_.height()};
                offset = pastePos_.second - center;
            }
            const QPointF p = snapToGrid(utilqt::toQPoint(offset));
            return ivec2{static_cast<int>(p.x()), static_cast<int>(p.y())};
        };

        std::stringstream ss;
        for (auto d : data) ss << d;
        // Activate the default context, might be needed in processor constructors.
        RenderContext::getPtr()->activateDefaultRenderContext();
        auto added = util::appendPartialProcessorNetwork(
            network_, ss, "", mainWindow_->getInviwoApplication(), offsetCallback);

        // Make sure the pasted processors are in the view
        auto selection = selectedItems();
        util::erase_remove_if(selection, [](auto i) {
            return qgraphicsitem_cast<ProcessorGraphicsItem*>(i) == nullptr;
        });
        if (!selection.empty()) {
            QRectF rect;
            for (auto item : selection) {
                rect = rect.united(item->sceneBoundingRect());
            }
            for (auto v : views()) {
                v->ensureVisible(rect);
            }
        }

    } catch (const Exception&) {
        LogWarn("Paste operation failed");
    }
}

void NetworkEditor::append(const std::filesystem::path& workspace) {
    NetworkLock lock(network_);
    try {
        const auto added =
            util::appendProcessorNetwork(network_, workspace, mainWindow_->getInviwoApplication());

        // Make sure the pasted processors are in the view
        auto selection = selectedItems();
        util::erase_remove_if(selection, [](auto i) {
            return qgraphicsitem_cast<ProcessorGraphicsItem*>(i) == nullptr;
        });

        QRectF rect;
        for (auto item : added) {
            auto pgi = getProcessorGraphicsItem(item);
            rect = rect.united(pgi->sceneBoundingRect());
        }
        for (auto v : views()) {
            v->ensureVisible(rect);
        }

    } catch (const Exception& e) {
        util::logError(e.getContext(), "Unable to append network {} due to {}", workspace,
                       e.getMessage());
    }

}  // namespace inviwo

void NetworkEditor::selectAll() {
    for (auto i : items()) i->setSelected(true);
}

QPointF NetworkEditor::snapToGrid(QPointF pos) {
    QPointF result;

    float ox = pos.x() > 0.0f ? 0.5f : -0.5f;
    float oy = pos.y() > 0.0f ? 0.5f : -0.5f;

    result.setX((int(pos.x() / gridSpacing_ + ox)) * gridSpacing_);
    result.setY((int(pos.y() / gridSpacing_ + oy)) * gridSpacing_);
    return result;
}

void NetworkEditor::drawBackground(QPainter* painter, const QRectF& rect) {
    if (!backgroundVisible_) {
        return;
    }

    painter->save();
    painter->setWorldMatrixEnabled(true);
    painter->fillRect(rect, QColor(0x7d, 0x80, 0x83));
    qreal left = int(rect.left()) - (int(rect.left()) % gridSpacing_);
    qreal top = int(rect.top()) - (int(rect.top()) % gridSpacing_);
    QVarLengthArray<QLineF, 100> linesX;
    painter->setPen(QColor(153, 153, 153));

    for (qreal x = left; x < rect.right(); x += gridSpacing_)
        linesX.append(QLineF(x, rect.top(), x, rect.bottom()));

    QVarLengthArray<QLineF, 100> linesY;

    for (qreal y = top; y < rect.bottom(); y += gridSpacing_)
        linesY.append(QLineF(rect.left(), y, rect.right(), y));

    painter->drawLines(linesX.data(), static_cast<int>(linesX.size()));
    painter->drawLines(linesY.data(), static_cast<int>(linesY.size()));
    painter->restore();
}

void NetworkEditor::drawForeground(QPainter* /*painter*/, const QRectF& /*rect*/) {
    // For testing purposes only. Draw bounding rects around all graphics items
    /*
    QList<QGraphicsItem*> items = QGraphicsScene::items(Qt::DescendingOrder);
    painter->setPen(Qt::magenta);
    for (QList<QGraphicsItem*>::iterator it = items.begin(); it != items.end(); ++it) {
        QRectF br = (*it)->sceneBoundingRect();
        painter->drawRect(br);
    }
    painter->setPen(Qt::red);
    painter->drawRect(QGraphicsScene::itemsBoundingRect());
    */
}

void NetworkEditor::initiateConnection(ProcessorOutportGraphicsItem* item) {
    processorItem_ = nullptr;
    automation_.leave();
    const auto pos = item->mapToScene(item->rect().center());
    const auto color = item->getPort()->getColorCode();
    connectionOutDragHelper_->start(item, pos, color);
}

void NetworkEditor::initiateConnection(ProcessorInportGraphicsItem* item) {
    processorItem_ = nullptr;
    automation_.leave();
    const auto pos = item->mapToScene(item->rect().center());
    const auto color = item->getPort()->getColorCode();
    connectionInDragHelper_->start(item, pos, color);
}

void NetworkEditor::releaseConnection(ProcessorInportGraphicsItem* item) {
    processorItem_ = nullptr;
    automation_.leave();

    // remove the old connection and add a new connection curve to be connected.
    auto oldConnection = item->getConnections()[0];
    auto port = oldConnection->getOutportGraphicsItem();
    const auto pos = oldConnection->getEndPoint();
    const auto color = oldConnection->getOutport()->getColorCode();
    removeConnection(oldConnection);
    connectionOutDragHelper_->start(port, pos, color);
}

void NetworkEditor::initiateLink(ProcessorLinkGraphicsItem* item, QPointF pos) {
    processorItem_ = nullptr;
    automation_.leave();

    linkDragHelper_->start(item, pos);
}

void NetworkEditor::resetAllTimeMeasurements() {
#if IVW_PROFILING
    // Update the status items
    for (auto& elem : processorGraphicsItems_) {
        elem.second->resetTimeMeasurements();
    }
#endif
}

// Manage various tooltips.
void NetworkEditor::helpEvent(QGraphicsSceneHelpEvent* e) {
    QList<QGraphicsItem*> graphicsItems = items(e->scenePos());
    for (auto item : graphicsItems) {
        try {
            switch (item->type()) {
                case ProcessorGraphicsItem::Type:
                    qgraphicsitem_cast<ProcessorGraphicsItem*>(item)->showToolTip(e);
                    return;
                case ConnectionGraphicsItem::Type:
                    qgraphicsitem_cast<ConnectionGraphicsItem*>(item)->showToolTip(e);
                    return;
                case LinkConnectionGraphicsItem::Type:
                    qgraphicsitem_cast<LinkConnectionGraphicsItem*>(item)->showToolTip(e);
                    return;
                case ProcessorInportGraphicsItem::Type:
                    qgraphicsitem_cast<ProcessorInportGraphicsItem*>(item)->showToolTip(e);
                    return;
                case ProcessorOutportGraphicsItem::Type:
                    qgraphicsitem_cast<ProcessorOutportGraphicsItem*>(item)->showToolTip(e);
                    return;
            }
        } catch (const Exception& e) {
            util::log(e.getContext(), e.getFullMessage());
            return;
        }
    }
    QGraphicsScene::helpEvent(e);
}

void NetworkEditor::onProcessorNetworkDidAddProcessor(Processor* processor) {
    addProcessorRepresentations(processor);
}

void NetworkEditor::onProcessorNetworkWillRemoveProcessor(Processor* processor) {
    removeProcessorRepresentations(processor);
}

void NetworkEditor::onProcessorNetworkDidAddConnection(const PortConnection& connection) {
    addConnectionGraphicsItem(connection);
}
void NetworkEditor::onProcessorNetworkWillRemoveConnection(const PortConnection& connection) {
    removeConnectionGraphicsItem(connection);
}

void NetworkEditor::onProcessorNetworkDidAddLink(const PropertyLink& propertyLink) {
    Processor* p1 = propertyLink.getSource()->getOwner()->getProcessor();
    Processor* p2 = propertyLink.getDestination()->getOwner()->getProcessor();
    auto link = getLinkGraphicsItem(p1, p2);
    if (!link) {
        addLinkGraphicsItem(p1, p2);
    }
}

void NetworkEditor::onProcessorNetworkDidRemoveLink(const PropertyLink& propertyLink) {
    if (network_
            ->getLinksBetweenProcessors(propertyLink.getSource()->getOwner()->getProcessor(),
                                        propertyLink.getDestination()->getOwner()->getProcessor())
            .size() == 0) {
        removeLinkGraphicsItem(
            getLinkGraphicsItem(propertyLink.getSource()->getOwner()->getProcessor(),
                                propertyLink.getDestination()->getOwner()->getProcessor()));
    }
}

NetworkEditor::AdjustSceneToChangesBlocker::AdjustSceneToChangesBlocker(NetworkEditor& network)
    : network_(network) {
    network_.adjustSceneToChange_ = false;
}

NetworkEditor::AdjustSceneToChangesBlocker::~AdjustSceneToChangesBlocker() {
    network_.adjustSceneToChange_ = true;
    network_.updateSceneSize();
}

}  // namespace inviwo
