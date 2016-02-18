/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/ports/portinspectorfactory.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/processors/processorwidgetfactory.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/resources/resourcemanager.h>
#include <inviwo/core/util/factory.h>
#include <inviwo/core/util/settings/linksettings.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/editor/linkdialog/linkdialog.h>
#include <inviwo/qt/editor/linkgraphicsitem.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/qt/editor/processorlinkgraphicsitem.h>
#include <inviwo/qt/editor/processorlistwidget.h>
#include <inviwo/qt/editor/processorportgraphicsitem.h>
#include <inviwo/qt/editor/processorprogressgraphicsitem.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/qt/editor/processorstatusgraphicsitem.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>
#include <inviwo/qt/widgets/propertylistwidget.h>
#include <inviwo/qt/widgets/eventconverterqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QApplication>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QVarLengthArray>
#include <QTimer>
#include <QPainter>
#include <QMimeData>
#include <warn/pop>

namespace inviwo {

const int NetworkEditor::GRID_SPACING = 25;

NetworkEditor::NetworkEditor(InviwoMainWindow* mainwindow)
    : QGraphicsScene()
    , oldConnectionTarget_(nullptr)
    , oldProcessorTarget_(nullptr)
    , connectionCurve_(nullptr)
    , linkCurve_(nullptr)
    , mainwindow_(mainwindow)
    , network_(mainwindow->getInviwoApplication()->getProcessorNetwork())
    , filename_("")
    , modified_(false) {
    network_->addObserver(this);

    // The defalt bsp tends to crash...
    setItemIndexMethod(QGraphicsScene::NoIndex);
    
    connect(this, &QGraphicsScene::selectionChanged, [&](){
        auto actions = mainwindow_->getActions();
        auto enable = selectedItems().size()>0;
        actions["Copy"]->setEnabled(enable);
        actions["Cut"]->setEnabled(enable);
        actions["Delete"]->setEnabled(enable);
    });
}

NetworkEditor::~NetworkEditor() {}

////////////////////////////////////////////////////////
//   PRIVATE METHODS FOR ADDING/REMOVING PROCESSORS   //
////////////////////////////////////////////////////////
ProcessorGraphicsItem* NetworkEditor::addProcessorRepresentations(Processor* processor) {
    // generate GUI representations (graphics item, property widget, processor widget)
    ProcessorGraphicsItem* ret = addProcessorGraphicsItem(processor);

    if (auto widget = processor->getProcessorWidget()){
        widget->addObserver(ret->getStatusItem());
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
    return processorGraphicsItem;
}

void NetworkEditor::removeProcessorGraphicsItem(Processor* processor) {
    // obtain processor graphics item through processor
    auto processorGraphicsItem = getProcessorGraphicsItem(processor);

    if (oldProcessorTarget_ == processorGraphicsItem) oldProcessorTarget_ = nullptr;

    removeItem(processorGraphicsItem);
    processorGraphicsItems_.erase(processor);
    // delete processor graphics item
    delete processorGraphicsItem;
}

void NetworkEditor::addPropertyWidgets(Processor* processor) {
    QCoreApplication::postEvent(
        mainwindow_->getPropertyListWidget(),
        new PropertyListEvent(PropertyListEvent::ADD, processor->getIdentifier()),
        Qt::LowEventPriority);
}

void NetworkEditor::removePropertyWidgets(Processor* processor) {
    QCoreApplication::postEvent(
        mainwindow_->getPropertyListWidget(),
        new PropertyListEvent(PropertyListEvent::REMOVE, processor->getIdentifier()),
        Qt::LowEventPriority);
}

void NetworkEditor::removeAndDeletePropertyWidgets(Processor* processor) {
    // Will not use events here since we might delete the processor
    mainwindow_->getPropertyListWidget()->removeAndDeleteProcessorProperties(processor);
}

ConnectionGraphicsItem* NetworkEditor::addConnectionGraphicsItem(PortConnection* connection) {
    Outport* outport = connection->getOutport();
    Inport* inport = connection->getInport();

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
    network_->removeConnection(outport, inport);
}

void NetworkEditor::removeConnectionGraphicsItem(PortConnection* connection) {
    ConnectionGraphicsItem* connectionGraphicsItem = connectionGraphicsItems_[connection];
    if (oldConnectionTarget_ == connectionGraphicsItem) oldConnectionTarget_ = nullptr;
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

    NetworkLock lock(network_);
    for (auto& link : links) {
        network_->removeLink(link->getSourceProperty(), link->getDestinationProperty());
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
    LinkDialog dialog(processor1, processor2, mainwindow_);
    dialog.exec();
}

//////////////////////////////////////
//   PORT INSPECTOR FUNCTIONALITY   //
//////////////////////////////////////

bool NetworkEditor::addPortInspector(Outport* port, QPointF pos) {
    if (!port) return false;

    auto it = portInspectors_.find(port);
    if (it != portInspectors_.end()) {
        return false;
    }

    auto factory = mainwindow_->getInviwoApplication()->getPortInspectorFactory();
    PortInspector* portInspector = factory->createAndCache(port->getClassIdentifier());

    portInspectors_[port] = portInspector;

    if (portInspector && !portInspector->isActive()) {
        portInspector->setActive(true);
        NetworkLock lock(network_);
        // Add processors to the network
        CanvasProcessor* canvasProcessor = portInspector->getCanvasProcessor();
        for (auto& processor : portInspector->getProcessors()) {
            network_->addProcessor(processor);
        }

        // Connect the port to inspect to the inports of the inspector network
        Outport* outport = dynamic_cast<Outport*>(port);
        for (auto& inport : portInspector->getInports()) {
            network_->addConnection(outport, inport);
        }

        // Add connections to the network
        for (auto& connection : portInspector->getConnections()) {
            network_->addConnection(connection->getOutport(), connection->getInport());
        }

        // Add links to the network
        for (auto& link : portInspector->getPropertyLinks()) {
            network_->addLink(link->getSourceProperty(), link->getDestinationProperty());
        }

        // Do autolinking.
        for (auto& processor : portInspector->getProcessors()) {
            network_->autoLinkProcessor(processor);
        }

        // Setup the widget
        if (auto processorWidget = canvasProcessor->getProcessorWidget()) {
            int size = mainwindow_->getInviwoApplication()
                           ->getSettingsByType<SystemSettings>()
                           ->portInspectorSize_.get();
            processorWidget->setDimensions(ivec2(size, size));
            if (auto widget = dynamic_cast<QWidget*>(processorWidget)) {
                widget->setMinimumSize(size, size);
                widget->setMaximumSize(size, size);
                widget->setWindowFlags(Qt::CustomizeWindowHint | Qt::Tool);
            }
            processorWidget->setPosition(ivec2(pos.x(), pos.y()));
            processorWidget->show();
            processorWidget->addObserver(new PortInspectorObserver(this, outport));
        }
        return true;
    }
    return false;
}

void NetworkEditor::removePortInspector(Outport* port) {
    if (port) {
        auto it = portInspectors_.find(port);
        if (it != portInspectors_.end()) {
            if (it->second && it->second->isActive()) {
                NetworkLock lock(network_);

                it->second->getCanvasProcessor()->getProcessorWidget()->hide();

                // Remove processors from the network
                auto processors = it->second->getProcessors();
                for (auto& processor : processors) {
                    network_->removeProcessor(processor);
                }
                it->second->setActive(false);
            }
            portInspectors_.erase(it);
        }
    }
}

std::unique_ptr<std::vector<unsigned char>> NetworkEditor::renderPortInspectorImage(
    Outport* outport, std::string& type) {
    std::unique_ptr<std::vector<unsigned char>> data;

    try {
        auto factory = mainwindow_->getInviwoApplication()->getPortInspectorFactory();
        auto portInspector = factory->createAndCache(outport->getClassIdentifier());

        if (portInspector && !portInspector->isActive()) {
            portInspector->setActive(true);

            auto canvasProcessor = portInspector->getCanvasProcessor();
            {
                NetworkLock lock(network_);
                // Add processors to the network
                for (auto& processor : portInspector->getProcessors()) {
                    network_->addProcessor(processor);
                    //RenderContext::getPtr()->activateDefaultRenderContext();
                }

                // Connect the port to inspect to the inports of the inspector network
                for (auto inport : portInspector->getInports()) {
                    network_->addConnection(outport, inport);
                }

                // Add connections to the network
                for (auto connection : portInspector->getConnections()) {
                    network_->addConnection(connection->getOutport(), connection->getInport());
                }

                // Add links to the network
                for (auto& link : portInspector->getPropertyLinks()) {
                    network_->addLink(link->getSourceProperty(), link->getDestinationProperty());
                }

                // Do auto-linking.
                for (auto processor : portInspector->getProcessors()) {
                    network_->autoLinkProcessor(processor);
                }

                int size = mainwindow_->getInviwoApplication()
                               ->getSettingsByType<SystemSettings>()
                               ->portInspectorSize_.get();
                canvasProcessor->setCanvasSize(ivec2(size, size));
            }  // Network will unlock and evaluate here.

            data = canvasProcessor->getVisibleLayerAsCodedBuffer(type);

            // remove the network...
            NetworkLock lock(network_);
            for (auto processor : portInspector->getProcessors()) {
                network_->removeProcessor(processor);
            }
            portInspector->setActive(false);
        }
    } catch (Exception& exception) {
        util::log(exception.getContext(), exception.getMessage(), LogLevel::Error);
    } catch (...) {
        util::log(IvwContext, "Problem using port inspector", LogLevel::Error);
    }
    return data;
}

bool NetworkEditor::isModified() const { return modified_; }
void NetworkEditor::setModified(const bool modified) {
    if (modified != modified_) {
        modified_ = modified;
        for (auto o : observers_) o->onModifiedStatusChanged(modified);
    }
}

////////////////////////////////////////////////////////
//   LOAD AND GET SNAPSHOT FROM EXTERNAL NETWORK      //
////////////////////////////////////////////////////////

void NetworkEditor::addExternalNetwork(std::string fileName, std::string identifierPrefix,
                                       ivec2 pos, unsigned int networkEditorFlags,
                                       ivec2 canvasSize) {
    NetworkLock lock(network_);

    auto app = mainwindow_->getInviwoApplication();
    Deserializer xmlDeserializer(app, fileName);
    ProcessorNetwork* processorNetwork = new ProcessorNetwork(app);
    processorNetwork->deserialize(xmlDeserializer);

    for (auto& processor : processorNetwork->getProcessors()) {
        std::string newIdentifier = identifierPrefix + "_" + processor->getIdentifier();
        processor->setIdentifier(newIdentifier);
        auto meta = processor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
        meta->setPosition(meta->getPosition() + pos);
        network_->addProcessor(processor);
    }

    for (auto connection : processorNetwork->getConnections()) {
        Outport* outport = connection->getOutport();
        Inport* inport = connection->getInport();
        // first remove the connection from the loaded network to avoid an already connected warning
        processorNetwork->removeConnection(outport, inport);
        network_->addConnection(outport, inport);
    }

    for (auto& link : processorNetwork->getLinks()) {
        network_->addLink(link->getSourceProperty(), link->getDestinationProperty());
    }
}

std::vector<std::string> NetworkEditor::saveSnapshotsInExternalNetwork(
    std::string externalNetworkFile, std::string identifierPrefix) {
    // turnoff sound
    auto app = mainwindow_->getInviwoApplication();
    BoolProperty* soundProperty =
        dynamic_cast<BoolProperty*>(app->getSettingsByType<SystemSettings>()
                                        ->getPropertyByIdentifier("enableSound"));
    bool isSoundEnabled = soundProperty->get();

    if (isSoundEnabled) soundProperty->set(false);

    std::vector<std::string> canvasSnapShotFiles;
    std::string directory = filesystem::getFileDirectory(externalNetworkFile);
    std::string workSpaceName = filesystem::getFileNameWithExtension(externalNetworkFile);
    std::string newFileName = filesystem::replaceFileExtension(workSpaceName, "png");

    for (auto& processor : network_->getProcessors()) {
        if (processor->getIdentifier().find(identifierPrefix) != std::string::npos) {
            CanvasProcessor* canvasProcessor = dynamic_cast<CanvasProcessor*>(processor);

            if (canvasProcessor) {
                std::string snapShotFilePath =
                    directory + "/" + "snapshot_" + canvasProcessor->getIdentifier() + newFileName;
                canvasSnapShotFiles.push_back(snapShotFilePath);
                canvasProcessor->saveImageLayer(snapShotFilePath);
            }
        }
    }

    if (isSoundEnabled) soundProperty->set(true);

    return canvasSnapShotFiles;
}

void NetworkEditor::removeExternalNetwork(std::string identifierPrefix) {
    NetworkLock lock(network_);

    for (auto& processor : network_->getProcessors()) {
        if (processor->getIdentifier().find(identifierPrefix) != std::string::npos)
            network_->removeProcessor(processor);
    }
}

std::vector<std::string> NetworkEditor::getSnapshotsOfExternalNetwork(std::string fileName) {
    std::vector<std::string> snapshotFileNames;
    // load external network
    QRectF rect = sceneRect();
    ivec2 pos(rect.width() / 2, rect.height() / 2);
    std::string identifierPrefix = "TemporaryExternalNetwork";
    unsigned int networkEditorFlags =
        NetworkEditor::UseOriginalCanvasSize | NetworkEditor::CanvasHidden;
    addExternalNetwork(fileName, identifierPrefix, pos, networkEditorFlags);
    network_->setModified(true);
    mainwindow_->getInviwoApplication()->getProcessorNetworkEvaluator()->requestEvaluate();
    // save snapshot
    snapshotFileNames = saveSnapshotsInExternalNetwork(fileName, identifierPrefix);
    // unload external network
    removeExternalNetwork(identifierPrefix);
    return snapshotFileNames;
}

////////////////////////////////////////////
//   OBTAIN GRAPHICS ITEMS FROM NETWORK   //
////////////////////////////////////////////
ProcessorGraphicsItem* NetworkEditor::getProcessorGraphicsItem(Processor* key) const {
    return util::map_find_or_null(processorGraphicsItems_, key);
}

ConnectionGraphicsItem* NetworkEditor::getConnectionGraphicsItem(PortConnection* key) const {
    return util::map_find_or_null(connectionGraphicsItems_, key);
}

LinkConnectionGraphicsItem* NetworkEditor::getLinkGraphicsItem(ProcessorPair key) const {
    return util::map_find_or_null(linkGraphicsItems_, key);
}

LinkConnectionGraphicsItem* NetworkEditor::getLinkGraphicsItem(Processor* processor1,
                                                               Processor* processor2) const {
    return getLinkGraphicsItem(ProcessorPair(processor1, processor2));
}

ProcessorGraphicsItem* NetworkEditor::getProcessorGraphicsItemAt(const QPointF pos) const {
    return getGraphicsItemAt<ProcessorGraphicsItem>(pos);
}
ProcessorInportGraphicsItem* NetworkEditor::getProcessorInportGraphicsItemAt(
    const QPointF pos) const {
    return getGraphicsItemAt<ProcessorInportGraphicsItem>(pos);
}
ConnectionGraphicsItem* NetworkEditor::getConnectionGraphicsItemAt(const QPointF pos) const {
    return getGraphicsItemAt<ConnectionGraphicsItem>(pos);
}
LinkConnectionGraphicsItem* NetworkEditor::getLinkGraphicsItemAt(const QPointF pos) const {
    return getGraphicsItemAt<LinkConnectionGraphicsItem>(pos);
}

////////////////////////////////
//   EVENT HANDLING METHODS   //
////////////////////////////////
void NetworkEditor::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    QGraphicsScene::mousePressEvent(e);
}

void NetworkEditor::mouseMoveEvent(QGraphicsSceneMouseEvent* e) {
    if (connectionCurve_) {
        // Connection drag mode
        connectionCurve_->setEndPoint(e->scenePos());
        connectionCurve_->reactToPortHover(getProcessorInportGraphicsItemAt(e->scenePos()));
        e->accept();

    } else if (linkCurve_) {
        // Link drag mode
        linkCurve_->setEndPoint(e->scenePos());
        linkCurve_->reactToProcessorHover(getProcessorGraphicsItemAt(e->scenePos()));
        e->accept();
    }

    QGraphicsScene::mouseMoveEvent(e);
}

void NetworkEditor::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
    for (auto& elem : processorGraphicsItems_) {
        QPointF pos = elem.second->pos();
        QPointF newpos = snapToGrid(pos);
        if (pos != newpos) elem.second->setPos(newpos);
    }

    if (connectionCurve_) {
        // connection drag mode
        Outport* startPort = connectionCurve_->getOutportGraphicsItem()->getPort();

        delete connectionCurve_;
        connectionCurve_ = nullptr;

        auto endItem = getProcessorInportGraphicsItemAt(e->scenePos());
        if (endItem && endItem->getPort()->canConnectTo(startPort)) {
            Inport* endPort = endItem->getPort();

            if (endPort->getNumberOfConnections() >= endPort->getMaxNumberOfConnections()) {
                network_->removeConnection(endPort->getConnectedOutport(), endPort);
            }
            network_->addConnection(startPort, endPort);
        }
        e->accept();

    } else if (linkCurve_) {
        // link drag mode
        Processor* startProcessor = linkCurve_->getSrcProcessorLinkGraphicsItem()
                                        ->getProcessorGraphicsItem()
                                        ->getProcessor();

        delete linkCurve_;
        linkCurve_ = nullptr;

        if (auto endProcessorItem = getProcessorGraphicsItemAt(e->scenePos())) {
            Processor* endProcessor = endProcessorItem->getProcessor();
            if (startProcessor != endProcessor) {
                showLinkDialog(startProcessor, endProcessor);
            }
        }
        e->accept();
    }

    QGraphicsScene::mouseReleaseEvent(e);
}

void NetworkEditor::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) {
    if (auto p = getProcessorGraphicsItemAt(e->scenePos())) {
        Processor* processor = p->getProcessor();
        if (processor && processor->hasProcessorWidget()) {
            if (processor->getProcessorWidget()->isVisible()) {
                processor->getProcessorWidget()->hide();
            } else {
                processor->getProcessorWidget()->show();
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

    if (!keyEvent->isAccepted()) {
        KeyboardEvent pressKeyEvent(EventConverterQt::getKeyButton(keyEvent),
                                    EventConverterQt::getModifier(keyEvent),
                                    KeyboardEvent::KEY_STATE_PRESS);

        progagateEventToSelecedProcessors(pressKeyEvent);
    }
}

void NetworkEditor::keyReleaseEvent(QKeyEvent* keyEvent) {
    QGraphicsScene::keyPressEvent(keyEvent);

    if (!keyEvent->isAccepted()) {
        KeyboardEvent releaseKeyEvent(EventConverterQt::getKeyButton(keyEvent),
                                      EventConverterQt::getModifier(keyEvent),
                                      KeyboardEvent::KEY_STATE_RELEASE);

        progagateEventToSelecedProcessors(releaseKeyEvent);
    }
}

void NetworkEditor::contextMenuEvent(QGraphicsSceneContextMenuEvent* e) {
    QMenu menu;

    for (auto& item : items(e->scenePos())) {
        if (auto outport = qgraphicsitem_cast<ProcessorOutportGraphicsItem*>(item)) {
            QAction* showPortInsector = menu.addAction(tr("Port Inspector"));
            showPortInsector->setCheckable(true);
            if (portInspectors_.find(outport->getPort()) != portInspectors_.end()) {
                showPortInsector->setChecked(true);
            }
            connect(showPortInsector, &QAction::triggered,
                    [&]() { contextMenuShowInspector(outport); });
            break;
        }

        if (auto inport = qgraphicsitem_cast<ProcessorInportGraphicsItem*>(item)) {
            QAction* showPortInsector = menu.addAction(tr("Port Inspector"));
            showPortInsector->setCheckable(true);
            if (portInspectors_.find(inport->getPort()->getConnectedOutport()) !=
                portInspectors_.end()) {
                showPortInsector->setChecked(true);
            }
            connect(showPortInsector, &QAction::triggered,
                    [&]() { contextMenuShowInspector(inport); });
            break;
        }

        if (auto processor = qgraphicsitem_cast<ProcessorGraphicsItem*>(item)) {
            QAction* renameAction = menu.addAction(tr("Rename Processor"));
            connect(renameAction, &QAction::triggered, [&]() {
                clearSelection();
                processor->setSelected(true);
                processor->editProcessorName();

            });

#if IVW_PROFILING
            QAction* resetTimeMeasurementsAction = menu.addAction(tr("Reset Time Measurements"));
            connect(resetTimeMeasurementsAction, &QAction::triggered,
                    [&]() { processor->resetTimeMeasurements(); });
#endif

            if (processor->getProcessor()->hasProcessorWidget()) {
                QAction* showAction = menu.addAction(tr("Show Widget"));
                showAction->setCheckable(true);
                if (auto processorWidget = processor->getProcessor()->getProcessorWidget()) {
                    showAction->setChecked(processorWidget->isVisible());
                }
                connect(showAction, &QAction::triggered, [&]() {
                    if (processor->getProcessor()->getProcessorWidget()->isVisible()) {
                        processor->getProcessor()->getProcessorWidget()->hide();
                    } else {
                        processor->getProcessor()->getProcessorWidget()->show();
                    }
                });
            }
            break;
        }

        if (auto connection = qgraphicsitem_cast<ConnectionGraphicsItem*>(item)) {
            QAction* showPortInsector = menu.addAction(tr("Show Port Inspector"));
            showPortInsector->setCheckable(true);
            if (portInspectors_.find(connection->getOutportGraphicsItem()->getPort()) !=
                portInspectors_.end()) {
                showPortInsector->setChecked(true);
            }
            connect(showPortInsector, &QAction::triggered,
                    [&]() { contextMenuShowInspector(connection->getOutportGraphicsItem()); });
            break;
        }

        if (auto link = qgraphicsitem_cast<LinkConnectionGraphicsItem*>(item)) {
            QAction* editLink = menu.addAction(tr("Edit Links"));
            connect(editLink, &QAction::triggered, [&]() {
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
        connect(editLink, &QAction::triggered, [&]() {
            showLinkDialog(selectedProcessors[0].second->getProcessor(),
                           selectedProcessors[1].second->getProcessor());
        });
    }

    menu.addSeparator();
    auto actions = mainwindow_->getActions();
    menu.addAction(actions["Copy"]);
    menu.addAction(actions["Cut"]);
    menu.addAction(actions["Paste"]);
    menu.addSeparator();

    QAction* deleteAction = actions["Delete"];
    menu.addAction(deleteAction);
    toBeDeleted_.append(items(e->scenePos()));
  
    
    doingContextMenu_ = true;
    menu.exec(QCursor::pos());
    e->accept();
    toBeDeleted_.clear();
    doingContextMenu_ = false;
}
bool NetworkEditor::doingContextMenu() const {
    return doingContextMenu_;
}

void NetworkEditor::progagateEventToSelecedProcessors(KeyboardEvent& pressKeyEvent) {
    for (auto& item : selectedItems()) {
        if (auto pgi = qgraphicsitem_cast<ProcessorGraphicsItem*>(item)) {
            Processor* p = pgi->getProcessor();
            p->propagateEvent(&pressKeyEvent, nullptr);
            if (pressKeyEvent.hasBeenUsed()) break;
        }
    }
}

void NetworkEditor::deleteSelection() {
    deleteItems(toBeDeleted_);
    toBeDeleted_.clear();
    deleteItems(selectedItems());
}

void NetworkEditor::deleteItems(QList<QGraphicsItem*> items) {
    NetworkLock lock(network_);

    // Remove Connections
    util::erase_remove_if(items, [&](QGraphicsItem* item){
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
        auto pgi = qgraphicsitem_cast<ProcessorGraphicsItem*>(item);
        if (pgi && !pgi->isEditingProcessorName()) {
            network_->removeAndDeleteProcessor(pgi->getProcessor());
            return true;
        } else {
            return false;
        }
    });
}



/////////////////////////////////////////
//   PROCESSOR DRAG AND DROP METHODS   //
/////////////////////////////////////////
void NetworkEditor::dragEnterEvent(QGraphicsSceneDragDropEvent* e) { dragMoveEvent(e); }

void NetworkEditor::dragMoveEvent(QGraphicsSceneDragDropEvent* e) {
    if (ProcessorDragObject::canDecode(e->mimeData())) {
        e->setAccepted(true);
        e->acceptProposedAction();
        ConnectionGraphicsItem* connectionItem = getConnectionGraphicsItemAt(e->scenePos());

        if (connectionItem && !oldConnectionTarget_) {  //< New connection found
            QString className;
            ProcessorDragObject::decode(e->mimeData(), className);

            try {
                auto factory = mainwindow_->getInviwoApplication()->getProcessorFactory();
                auto processor = factory->create(className.toLocal8Bit().constData());

                bool inputmatch =
                    util::any_of(processor->getInports(), [&connectionItem](Inport* inport) {
                        return inport->canConnectTo(connectionItem->getOutport());
                    });
                bool outputmatch =
                    util::any_of(processor->getOutports(), [&connectionItem](Outport* outport) {
                        return connectionItem->getInport()->canConnectTo(outport);
                    });

                if (inputmatch && outputmatch) {
                    connectionItem->setBorderColor(Qt::green);
                } else {
                    connectionItem->setBorderColor(Qt::red);
                }
                connectionItem->setMidPoint(e->scenePos());
            } catch (Exception&) {
                connectionItem->setBorderColor(Qt::red);
            }
            oldConnectionTarget_ = connectionItem;

        } else if (connectionItem) {  // move event on active connection
            oldConnectionTarget_->setMidPoint(e->scenePos());

        } else if (oldConnectionTarget_ && !connectionItem) {  //< Connection no longer targeted
            oldConnectionTarget_->resetBorderColors();
            oldConnectionTarget_->clearMidPoint();
            oldConnectionTarget_ = nullptr;

        } else if (!connectionItem) {  // processor replacement
            ProcessorGraphicsItem* processorItem = getProcessorGraphicsItemAt(e->scenePos());

            if (processorItem && !oldProcessorTarget_) {  //< New processor found
                QString className;
                ProcessorDragObject::decode(e->mimeData(), className);
                processorItem->setHighlight(true);
                oldProcessorTarget_ = processorItem;
            } else if (!processorItem && oldProcessorTarget_) {  // processor no longer targeted
                oldProcessorTarget_->setHighlight(false);
                oldProcessorTarget_ = nullptr;
            }
        }
    }
}

void NetworkEditor::dropEvent(QGraphicsSceneDragDropEvent* e) {
    if (ProcessorDragObject::canDecode(e->mimeData())) {
        QString name;
        ProcessorDragObject::decode(e->mimeData(), name);
        std::string className = name.toLocal8Bit().constData();

        NetworkLock lock(network_);
        
        if (!className.empty()) {
            e->setAccepted(true);
            e->acceptProposedAction();

            try {
                // create processor, add it to processor network, and generate it's widgets
                auto factory = mainwindow_->getInviwoApplication()->getProcessorFactory();
                Processor* processor = factory->create(className).release();

                clearSelection();

                ProcessorMetaData* meta =
                    processor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);

                if (oldProcessorTarget_) {
                    meta->setPosition(vec2(oldProcessorTarget_->scenePos().x(),
                                           oldProcessorTarget_->scenePos().y()));
                } else {
                    QPointF spos = snapToGrid(e->scenePos());
                    meta->setPosition(vec2(spos.x(), spos.y()));
                }

                network_->addProcessor(processor);
                network_->autoLinkProcessor(processor);

                if (oldConnectionTarget_) {
                    placeProcessorOnConnection(processor, oldConnectionTarget_);
                } else if (oldProcessorTarget_) {
                    placeProcessorOnProcessor(processor, oldProcessorTarget_->getProcessor());
                }
            } catch (Exception& exception) {
                if (oldConnectionTarget_) {
                    oldConnectionTarget_->resetBorderColors();
                    oldConnectionTarget_->clearMidPoint();
                }
                util::log(exception.getContext(), "Unable to create processor " + className +
                                                      " due to " + exception.getMessage(),
                          LogLevel::Error);
            }

            // clear oldDragTarget
            oldConnectionTarget_ = nullptr;
            oldProcessorTarget_ = nullptr;
        }
    }
}

void NetworkEditor::placeProcessorOnConnection(Processor* processor,
                                               ConnectionGraphicsItem* connectionItem) {
    Inport* connectionInport = connectionItem->getInport();
    Outport* connectionOutport = connectionItem->getOutport();

    Inport* inport = util::find_if_or_null(
        processor->getInports(),
        [connectionOutport](Inport* port) { return port->canConnectTo(connectionOutport); });

    Outport* outport = util::find_if_or_null(
        processor->getOutports(),
        [connectionInport](Outport* port) { return connectionInport->canConnectTo(port); });

    if (inport && outport) {
        NetworkLock lock(network_);
        // Remove old connection
        network_->removeConnection(connectionOutport, connectionInport);

        // Add new Connections
        network_->addConnection(connectionOutport, inport);
        network_->addConnection(outport, connectionInport);
    } else {
        connectionItem->resetBorderColors();
        connectionItem->clearMidPoint();
    }
}

void NetworkEditor::placeProcessorOnProcessor(Processor* newProcessor, Processor* oldProcessor) {
    const std::vector<Inport*>& inports = newProcessor->getInports();
    const std::vector<Outport*>& outports = newProcessor->getOutports();
    const std::vector<Inport*>& oldInports = oldProcessor->getInports();
    const std::vector<Outport*>& oldOutports = oldProcessor->getOutports();

    NetworkLock lock(network_);

    std::vector<std::pair<Outport*, Inport*>> newConnections;

    for (size_t i = 0; i < std::min(inports.size(), oldInports.size()); ++i) {
        for (auto outport : oldInports[i]->getConnectedOutports()) {
            if (inports[i]->canConnectTo(outport)) {
                // save new connection connectionOutportoldInport-processorInport
                newConnections.push_back(std::make_pair(outport, inports[i]));
            }
        }
    }

    for (size_t i = 0; i < std::min(outports.size(), oldOutports.size()); ++i) {
        for (auto inport : oldOutports[i]->getConnectedInports()) {
            if (inport->canConnectTo(outports[i])) {
                // save new connection processorOutport-connectionInport
                newConnections.push_back(std::make_pair(outports[i], inport));
            }
        }
    }

    // Copy over the value of old props to new ones if id and classname are equal.
    auto newProps = newProcessor->getProperties();
    auto oldProps = oldProcessor->getProperties();

    std::map<Property*, Property*> propertymap;

    for (auto newProp : newProps) {
        for (auto oldProp : oldProps) {
            if (newProp->getIdentifier() == oldProp->getIdentifier() &&
                newProp->getClassIdentifier() == oldProp->getClassIdentifier()) {
                newProp->set(oldProp);
                propertymap[oldProp] = newProp;
            }
        }
    }

    // Move property links to the new processor
    auto links = network_->getLinks();
    for (auto oldProp : oldProps) {
        for (auto link : links) {
            if (link->getDestinationProperty() == oldProp) {
                auto match = propertymap.find(oldProp);
                if (match != propertymap.end()) {
                    // add link from
                    Property* start = link->getSourceProperty();
                    // to
                    Property* end = match->second;

                    network_->addLink(start, end);
                }
            }
            if (link->getSourceProperty() == oldProp) {
                auto match = propertymap.find(oldProp);
                if (match != propertymap.end()) {
                    // add link from
                    Property* start = match->second;
                    // to
                    Property* end = link->getDestinationProperty();

                    network_->addLink(start, end);
                }
            }
        }
    }

    // remove old processor
    network_->removeProcessor(oldProcessor);
    delete oldProcessor;

    // create all new connections
    for (auto& newConnection : newConnections)
        network_->addConnection(newConnection.first, newConnection.second);
}

///////////////////////////////
//   SERIALIZATION METHODS   //
///////////////////////////////
void NetworkEditor::clearNetwork() {
    NetworkLock lock(network_);

    // We need to clear the portInspectors manually otherwise the pointers in the
    // PortInspector networks won't be cleared
    PortInspectorMap portInspectors = portInspectors_;
    for (auto& portInspector : portInspectors) removePortInspector(portInspector.first);

    network_->clear();
    ResourceManager::getPtr()->clearAllResources();

    setModified(true);
}

bool NetworkEditor::saveNetwork(std::string fileName) {
    try {
        Serializer xmlSerializer(fileName);
        network_->serialize(xmlSerializer);
        network_->setModified(false);
        xmlSerializer.writeFile();
        filename_ = fileName;
        setModified(false);
    } catch (SerializationException& exception) {
        util::log(exception.getContext(),
                  "Unable to save network " + fileName + " due to " + exception.getMessage(),
                  LogLevel::Error);
        return false;
    }
    return true;
}

bool NetworkEditor::saveNetwork(std::ostream stream) {
    try {
        Serializer xmlSerializer(filename_);
        network_->serialize(xmlSerializer);
        network_->setModified(false);
        xmlSerializer.writeFile(stream);
        setModified(false);
    } catch (SerializationException& exception) {
        util::log(exception.getContext(),
                  "Unable to save network " + filename_ + " due to " + exception.getMessage(),
                  LogLevel::Error);
        return false;
    }
    return true;
}

bool NetworkEditor::loadNetwork(std::string fileName) {
    std::ifstream fileStream(fileName.c_str());
    if (!fileStream) {
        LogError("Could not open workspace file: " << fileName);
        fileStream.close();
        return false;
    }
    bool loaded = loadNetwork(fileStream, fileName);
    fileStream.close();

    return loaded;
}

bool NetworkEditor::loadNetwork(std::istream& stream, const std::string& path) {
    // Clean the current network
    clearNetwork();
    {
        // Lock the network that so no evaluations are triggered during the de-serialization
        NetworkLock lock(network_);

        // Deserialize processor network
        try {
            Deserializer xmlDeserializer(mainwindow_->getInviwoApplication(), stream, path);
            network_->deserialize(xmlDeserializer);
        } catch (const AbortException& exception) {
            util::log(exception.getContext(),
                      "Unable to load network " + path + " due to " + exception.getMessage(),
                      LogLevel::Error);
            clearNetwork();
            return false;
        } catch (const IgnoreException& exception) {
            util::log(exception.getContext(),
                      "Incomplete network loading " + path + " due to " + exception.getMessage(),
                      LogLevel::Error);
        }
        network_->setModified(true);
        for (auto o : observers_) o->onNetworkEditorFileChanged(path);
        InviwoApplicationQt::processEvents();  // make sure the gui is ready before we unlock.
    }

    setModified(false);
    filename_ = path;
    return true;
}

bool NetworkEditor::event(QEvent* e) {
    if (e->type() == PortInspectorEvent::type()) {
        e->accept();
        auto pie = static_cast<PortInspectorEvent*>(e);
        removePortInspector(pie->port_);
        return true;
    }
    return QGraphicsScene::event(e);
}

QByteArray NetworkEditor::copy() const {
    std::stringstream ss;
    util::serializeSelected(network_, ss, "");
    auto str = ss.str();
    QByteArray byteArray(str.c_str(), static_cast<int>(str.length()));
    return byteArray;
}

QByteArray NetworkEditor::cut() {
    auto res = copy();
    deleteSelection();
    return res;
}

void NetworkEditor::paste(QByteArray mimeData) {
    std::stringstream ss;
    for (auto d : mimeData) ss << d;
    auto added = util::appendDeserialized(network_, ss, "", mainwindow_->getInviwoApplication());

    for (auto p : added) {
        auto m = p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
        m->setPosition(m->getPosition() + ivec2(50, 50));
    }
}

void NetworkEditor::selectAll() {
    for(auto i : items()) i->setSelected(true);
}

////////////////////////
//   HELPER METHODS   //
////////////////////////
QPointF NetworkEditor::snapToGrid(QPointF pos) {
    QPointF result;

    float ox = pos.x() > 0.0f ? 0.5f : -0.5f;
    float oy = pos.y() > 0.0f ? 0.5f : -0.5f;

    result.setX((int(pos.x() / GRID_SPACING + ox)) * GRID_SPACING);
    result.setY((int(pos.y() / GRID_SPACING + oy)) * GRID_SPACING);
    return result;
}

void NetworkEditor::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->save();
    painter->setWorldMatrixEnabled(true);
    painter->fillRect(rect, Qt::darkGray);
    qreal left = int(rect.left()) - (int(rect.left()) % GRID_SPACING);
    qreal top = int(rect.top()) - (int(rect.top()) % GRID_SPACING);
    QVarLengthArray<QLineF, 100> linesX;
    painter->setPen(QColor(153, 153, 153));

    for (qreal x = left; x < rect.right(); x += GRID_SPACING)
        linesX.append(QLineF(x, rect.top(), x, rect.bottom()));

    QVarLengthArray<QLineF, 100> linesY;

    for (qreal y = top; y < rect.bottom(); y += GRID_SPACING)
        linesY.append(QLineF(rect.left(), y, rect.right(), y));

    painter->drawLines(linesX.data(), linesX.size());
    painter->drawLines(linesY.data(), linesY.size());
    painter->restore();

    // For testing purpuses only. Draw bounding rects around all graphics items
    // QList<QGraphicsItem*> items = QGraphicsScene::items(Qt::DescendingOrder);
    // painter->setPen(Qt::magenta);
    // for (QList<QGraphicsItem*>::iterator it = items.begin(); it != items.end(); ++it) {
    //    QRectF br = (*it)->sceneBoundingRect();
    //    painter->drawRect(br);
    //}
    // painter->setPen(Qt::red);
    // painter->drawRect(QGraphicsScene::itemsBoundingRect());
}

void NetworkEditor::initiateConnection(ProcessorOutportGraphicsItem* item) {
    QPointF pos = item->mapToScene(item->rect().center());
    connectionCurve_ = new ConnectionDragGraphicsItem(item, pos, item->getPort()->getColorCode());
    addItem(connectionCurve_);
    connectionCurve_->show();
}

void NetworkEditor::releaseConnection(ProcessorInportGraphicsItem* item) {
    // remove the old connection and add a new connection curve to be connected.
    ConnectionGraphicsItem* oldConnection = item->getConnections()[0];
    connectionCurve_ = new ConnectionDragGraphicsItem(oldConnection->getOutportGraphicsItem(),
                                                      oldConnection->getEndPoint(),
                                                      oldConnection->getOutport()->getColorCode());
    removeConnection(oldConnection);
    addItem(connectionCurve_);
    connectionCurve_->show();
}

void NetworkEditor::initiateLink(ProcessorLinkGraphicsItem* item, QPointF pos) {
    linkCurve_ = new LinkConnectionDragGraphicsItem(item, pos);
    addItem(linkCurve_);
    linkCurve_->setZValue(DRAGING_ITEM_DEPTH);
    linkCurve_->show();
}

void NetworkEditor::updateLeds() {
    // Update the status items
    for (auto& elem : processorGraphicsItems_) {
        elem.second->getStatusItem()->update();
    }
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

            // case ProcessorLinkGraphicsItem::Type:
            //    qgraphicsitem_cast<ProcessorLinkGraphicsItem*>(item)->showToolTip(e);
            //    return;
            case ProcessorInportGraphicsItem::Type:
                qgraphicsitem_cast<ProcessorInportGraphicsItem*>(item)->showToolTip(e);
                return;
            case ProcessorOutportGraphicsItem::Type:
                qgraphicsitem_cast<ProcessorOutportGraphicsItem*>(item)->showToolTip(e);
                return;
                // case ProcessorStatusGraphicsItem::Type:
                //    qgraphicsitem_cast<ProcessorStatusGraphicsItem*>(item)->showToolTip(e);
                //    return;
                // case ProcessorProgressGraphicsItem::Type:
                //    qgraphicsitem_cast<ProcessorProgressGraphicsItem*>(item)->showToolTip(e);
                //    return;
        };
    }
    QGraphicsScene::helpEvent(e);
}

void NetworkEditor::contextMenuShowInspector(EditorGraphicsItem* item) {
    Outport* port = nullptr;
    QPointF pos;
    if (auto pin = qgraphicsitem_cast<ProcessorInportGraphicsItem*>(item)) {
        pos = pin->mapPosToSceen(pin->rect().center());
        port = pin->getPort()->getConnectedOutport();
    } else if (auto pout = qgraphicsitem_cast<ProcessorOutportGraphicsItem*>(item)) {
        pos = pout->mapPosToSceen(pout->rect().center());
        port = pout->getPort();
    } else {
        return;
    }

    if (portInspectors_.find(port) == portInspectors_.end()) {
        addPortInspector(port, pos);
    } else {
        removePortInspector(port);
    }
}

void NetworkEditor::onProcessorNetworkDidAddProcessor(Processor* processor) {
    setModified(true);
    addProcessorRepresentations(processor);
}

void NetworkEditor::onProcessorNetworkWillRemoveProcessor(Processor* processor) {
    setModified(true);

    for (auto& outport : processor->getOutports()) removePortInspector(outport);
    removeProcessorRepresentations(processor);
}

void NetworkEditor::onProcessorNetworkDidAddConnection(PortConnection* connection) {
    addConnectionGraphicsItem(connection);
    updateLeds();
    setModified(true);
}
void NetworkEditor::onProcessorNetworkWillRemoveConnection(PortConnection* connection) {
    removeConnectionGraphicsItem(connection);
    updateLeds();
    setModified(true);
}

void NetworkEditor::onProcessorNetworkDidAddLink(PropertyLink* propertyLink) {
    setModified(true);
    Processor* p1 = propertyLink->getSourceProperty()->getOwner()->getProcessor();
    Processor* p2 = propertyLink->getDestinationProperty()->getOwner()->getProcessor();
    auto link = getLinkGraphicsItem(p1, p2);
    if (!link) {
        addLinkGraphicsItem(p1, p2);
    }
}

void NetworkEditor::onProcessorNetworkDidRemoveLink(PropertyLink* propertyLink) {
    setModified(true);
    if (network_->getLinksBetweenProcessors(
                    propertyLink->getSourceProperty()->getOwner()->getProcessor(),
                    propertyLink->getDestinationProperty()->getOwner()->getProcessor())
            .size() == 0) {
        removeLinkGraphicsItem(getLinkGraphicsItem(
            propertyLink->getSourceProperty()->getOwner()->getProcessor(),
            propertyLink->getDestinationProperty()->getOwner()->getProcessor()));
    }
}

void NetworkEditor::onProcessorNetworkChange() {
    setModified();
    updateLeds();
}

void PortInspectorObserver::onProcessorWidgetHide(ProcessorWidget* widget) {
    widget->removeObserver(this);
    QCoreApplication::postEvent(editor_, new PortInspectorEvent(port_), Qt::LowEventPriority);
    delete this;
}
QEvent::Type PortInspectorEvent::PortInspectorEventType = QEvent::None;

}  // namespace
