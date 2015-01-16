/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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
#include <inviwo/core/io/serialization/ivwdeserializer.h>
#include <inviwo/core/io/serialization/ivwserializer.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/metadata/processorwidgetmetadata.h>
#include <inviwo/core/ports/geometryport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/multidatainport.h>
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
#include <inviwo/core/util/filesystem.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/editor/linkdialog/linkdialog.h>
#include <inviwo/qt/editor/linkgraphicsitem.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/qt/editor/processorlinkgraphicsitem.h>
#include <inviwo/qt/editor/processorlistwidget.h>
#include <inviwo/qt/editor/processorportgraphicsitem.h>
#include <inviwo/qt/editor/processorprogressgraphicsitem.h>
#include <inviwo/qt/editor/processorstatusgraphicsitem.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>
#include <inviwo/qt/widgets/propertylistwidget.h>
#include <inviwo/qt/widgets/eventconverterqt.h>

#include <QApplication>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QVarLengthArray>
#include <QTimer>
#include <QPainter>


namespace inviwo {

const int NetworkEditor::GRID_SPACING = 25;

NetworkEditor::NetworkEditor()
    : QGraphicsScene()
    , oldConnectionTarget_(NULL)
    , oldProcessorTarget_(NULL)
    , connectionCurve_(NULL)
    , linkCurve_(NULL)
    , filename_("")
    , modified_(false) {
    InviwoApplication::getPtr()->getProcessorNetwork()->addObserver(this);

    // The defalt bsp tends to crash...  
    setItemIndexMethod(QGraphicsScene::NoIndex);
}

NetworkEditor::~NetworkEditor() {}


////////////////////////////////////////////////////////
//   PRIVATE METHODS FOR ADDING/REMOVING PROCESSORS   //
////////////////////////////////////////////////////////
ProcessorGraphicsItem* NetworkEditor::addProcessorRepresentations(Processor* processor, QPointF pos,
                                                                  bool showProcessor,
                                                                  bool selectProcessor) {
    // generate GUI representations (graphics item, property widget, processor widget)
    ProcessorGraphicsItem* ret =
        addProcessorGraphicsItem(processor, pos, showProcessor, selectProcessor);

    ProcessorWidget* processorWidget = ProcessorWidgetFactory::getPtr()->create(processor);
    if (processorWidget) {
        processorWidget->setProcessor(processor);
        processor->setProcessorWidget(processorWidget);

        QWidget* widget = dynamic_cast<QWidget*>(processorWidget);
        if (widget) {
            InviwoApplicationQt* app =
                dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr());
            widget->setParent(app->getMainWindow());
        }
        processorWidget->initialize();
        processorWidget->setVisible(processorWidget->ProcessorWidget::isVisible()); 
        processorWidget->addObserver(ret->getStatusItem());
    }
    return ret;
}

void NetworkEditor::removeProcessorRepresentations(Processor* processor) {
    removeProcessorGraphicsItem(processor);
    removeAndDeletePropertyWidgets(processor);
    
    // processor widget should be removed here since it is added in addProcessorRepresentations()
    ProcessorWidget* processorWidget = processor->getProcessorWidget();
    if (processorWidget) {
        processorWidget->deinitialize();
        processor->setProcessorWidget(NULL);
        delete processorWidget;
    }
}

ProcessorGraphicsItem* NetworkEditor::addProcessorGraphicsItem(Processor* processor, QPointF pos,
                                                               bool visible, bool selected) {
    // generate GUI representation and add to editor
    ProcessorGraphicsItem* processorGraphicsItem = new ProcessorGraphicsItem(processor);
    processorGraphicsItem->setVisible(visible);
    processorGraphicsItem->setSelected(selected);
    processorGraphicsItem->setPos(pos);
    addItem(processorGraphicsItem);
    processorGraphicsItems_[processor] = processorGraphicsItem;

    return processorGraphicsItem;
}

void NetworkEditor::removeProcessorGraphicsItem(Processor* processor) {
    // obtain processor graphics item through processor
    ProcessorGraphicsItem* processorGraphicsItem = getProcessorGraphicsItem(processor);

    if (oldProcessorTarget_ == processorGraphicsItem) oldProcessorTarget_ = NULL;

    removeItem(processorGraphicsItem);
    processorGraphicsItems_.erase(processor);
    // delete processor graphics item
    delete processorGraphicsItem;
}

void NetworkEditor::addPropertyWidgets(Processor* processor) {
    QCoreApplication::postEvent(
        propertyListWidget_,
        new PropertyListEvent(PropertyListEvent::ADD, processor),
        Qt::LowEventPriority);
}

void NetworkEditor::removePropertyWidgets(Processor* processor) {
    QCoreApplication::postEvent(
        propertyListWidget_,
        new PropertyListEvent(PropertyListEvent::REMOVE, processor),
        Qt::LowEventPriority);
}

void NetworkEditor::removeAndDeletePropertyWidgets(Processor* processor) {
    // Will not use events here since we might delete the processor
    propertyListWidget_->removeAndDeleteProcessorProperties(processor);
}

void NetworkEditor::setPropertyListWidget(PropertyListWidget* widget) {
    propertyListWidget_ = widget;
}
PropertyListWidget* NetworkEditor::getPropertyListWidget() const { return propertyListWidget_; }

ConnectionGraphicsItem* NetworkEditor::addConnectionGraphicsItem(PortConnection* connection) {
    Outport* outport = connection->getOutport();
    Inport* inport = connection->getInport();

    ProcessorGraphicsItem* outProcessor = getProcessorGraphicsItem(outport->getProcessor());
    ProcessorGraphicsItem* inProcessor = getProcessorGraphicsItem(inport->getProcessor());

    ConnectionGraphicsItem* connectionGraphicsItem =
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
    InviwoApplication::getPtr()->getProcessorNetwork()->removeConnection(outport, inport);
}

void NetworkEditor::removeConnectionGraphicsItem(PortConnection* connection) {
    ConnectionGraphicsItem* connectionGraphicsItem = connectionGraphicsItems_[connection];
    if (oldConnectionTarget_ == connectionGraphicsItem) oldConnectionTarget_ = NULL;
    connectionGraphicsItems_.erase(connection);
    delete connectionGraphicsItem;
}


LinkConnectionGraphicsItem* NetworkEditor::addLinkGraphicsItem(Processor* processor1,
                                                               Processor* processor2) {

    if (processor1 && processor2) {
        ProcessorLinkGraphicsItem* p1 = getProcessorGraphicsItem(processor1)->getLinkGraphicsItem();
        ProcessorLinkGraphicsItem* p2 = getProcessorGraphicsItem(processor2)->getLinkGraphicsItem();

        LinkConnectionGraphicsItem* linkGraphicsItem = new LinkConnectionGraphicsItem(p1, p2);
        linkGraphicsItems_[ProcessorPair(processor1,processor2)] = linkGraphicsItem;
        addItem(linkGraphicsItem);
        return linkGraphicsItem;
    }
    return NULL;
}

void NetworkEditor::removeLink(LinkConnectionGraphicsItem* linkGraphicsItem) {
    ProcessorNetwork* network = InviwoApplication::getPtr()->getProcessorNetwork();
    ProcessorNetwork::PropertyLinkVector links = network->getLinksBetweenProcessors(
        linkGraphicsItem->getSrcProcessorGraphicsItem()->getProcessor(),
        linkGraphicsItem->getDestProcessorGraphicsItem()->getProcessor());

    network->lock();
    for (ProcessorNetwork::PropertyLinkVector::iterator it = links.begin(); it != links.end();
         ++it) {
        network->removeLink((*it)->getSourceProperty(), (*it)->getDestinationProperty());
    }
    network->unlock();
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
    InviwoApplicationQt* app = dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr());
    
    LinkDialog dialog(processor1, processor2, app->getMainWindow());
    dialog.exec();
}

//////////////////////////////////////
//   PORT INSPECTOR FUNCTIONALITY   //
//////////////////////////////////////

bool NetworkEditor::addPortInspector(Outport* port, QPointF pos) {
    if (!port) return false;

    PortInspectorMap::iterator it = portInspectors_.find(port);
    if (it != portInspectors_.end()) {
        return false;
    }

    ProcessorNetwork* network = InviwoApplication::getPtr()->getProcessorNetwork();
    PortInspector* portInspector =
        PortInspectorFactory::getPtr()->getPortInspectorForPortClass(port->getClassIdentifier());

    portInspectors_[port] = portInspector;

    if (portInspector && !portInspector->isActive()) {
        portInspector->setActive(true);
        network->lock();
        // Add processors to the network
        CanvasProcessor* canvasProcessor = portInspector->getCanvasProcessor();
        canvasProcessor->deinitialize();
        std::vector<Processor*> processors = portInspector->getProcessors();
        for (size_t i = 0; i < processors.size(); i++) {
            // For Debugging
            //ProcessorMetaData* meta = processors[i]->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
            //meta->setVisibile(true);
            network->addProcessor(processors[i]);
        }
        canvasProcessor->initialize(); // This is needed since, we need to call initialize after we
                                       // add the canvas to the processor.

        // Connect the port to inspect to the inports of the inspector network
        Outport* outport = dynamic_cast<Outport*>(port);
        std::vector<Inport*> inports = portInspector->getInports();
        for (size_t i = 0; i < inports.size(); i++) network->addConnection(outport, inports[i]);

        // Add connections to the network
        std::vector<PortConnection*> connections = portInspector->getConnections();
        for (size_t i = 0; i < connections.size(); i++) {
            network->addConnection(connections[i]->getOutport(), connections[i]->getInport());
        }

        // Add links to the network
        std::vector<PropertyLink*> links = portInspector->getPropertyLinks();
        for (size_t i = 0; i < links.size(); i++) {
            network->addLink(links[i]->getSourceProperty(), links[i]->getDestinationProperty());
        }

        // Do autolinking.
        for (size_t i = 0; i < processors.size(); i++) {
            network->autoLinkProcessor(processors[i]);
        }

        // Setup the widget
        ProcessorWidget* processorWidget = canvasProcessor->getProcessorWidget();
        if (processorWidget) {
            int size = InviwoApplication::getPtr()
                           ->getSettingsByType<SystemSettings>()
                           ->portInspectorSize_.get();
            processorWidget->setDimensions(ivec2(size,size));
            QWidget* widget = dynamic_cast<QWidget*>(processorWidget);
            if (widget) {
                widget->setMinimumSize(size, size);
                widget->setMaximumSize(size, size);
                widget->setWindowFlags(Qt::CustomizeWindowHint | Qt::Tool);
            }
            processorWidget->setPosition(ivec2(pos.x(), pos.y()));
            processorWidget->show();
            processorWidget->addObserver(new PortInspectorObserver(this, outport));
        }

        network->unlock();
        return true;
    }
    return false;
}

void NetworkEditor::removePortInspector(Outport* port) {
    if (port) {
        ProcessorNetwork* network = InviwoApplication::getPtr()->getProcessorNetwork();
        PortInspectorMap::iterator it = portInspectors_.find(port);
        if (it!= portInspectors_.end() && it->second->isActive()) {
            network->lock();
            // Remove processors from the network
            std::vector<Processor*> processors = it->second->getProcessors();
            for (size_t i = 0; i < processors.size(); i++) network->removeProcessor(processors[i]);
           
            network->unlock();
            it->second->setActive(false);
            portInspectors_.erase(it);
        }
    }
}

std::vector<unsigned char>* NetworkEditor::renderPortInspectorImage(Port* port, std::string type) {
    PortInspector* portInspector =
        PortInspectorFactory::getPtr()->getPortInspectorForPortClass(port->getClassIdentifier());

    ProcessorNetwork* network = InviwoApplication::getPtr()->getProcessorNetwork();
    std::vector<unsigned char>* data = NULL;

    if (portInspector && !portInspector->isActive()) {
        portInspector->setActive(true);
        network->lock();
        // Add processors to the network
        CanvasProcessor* canvasProcessor = portInspector->getCanvasProcessor();

        ProcessorWidgetMetaData* wm = canvasProcessor->createMetaData<ProcessorWidgetMetaData>(
            ProcessorWidgetMetaData::CLASS_IDENTIFIER);
        wm->setVisibile(false);

        std::vector<Processor*> processors = portInspector->getProcessors();
        for (size_t i = 0; i < processors.size(); i++) {
            network->addProcessor(processors[i]);
        }

        canvasProcessor->initialize(); // This is needed since, we need to call initialize after we
                                       // add the canvas to the processor.

        // Connect the port to inspect to the inports of the inspector network
        Outport* outport = dynamic_cast<Outport*>(port);
        std::vector<Inport*> inports = portInspector->getInports();
        for (size_t i = 0; i < inports.size(); i++) network->addConnection(outport, inports[i]);

        // Add connections to the network
        std::vector<PortConnection*> connections = portInspector->getConnections();
        for (size_t i = 0; i < connections.size(); i++) {
            network->addConnection(connections[i]->getOutport(), connections[i]->getInport());
        }

        // Add links to the network
        std::vector<PropertyLink*> links = portInspector->getPropertyLinks();
        for (size_t i = 0; i < links.size(); i++) {
            network->addLink(links[i]->getSourceProperty(), links[i]->getDestinationProperty());
        }

        // Do autolinking.
        for (size_t i = 0; i < processors.size(); i++) {
            network->autoLinkProcessor(processors[i]);
        }

        int size = InviwoApplication::getPtr()
                       ->getSettingsByType<SystemSettings>()
                       ->portInspectorSize_.get();
        canvasProcessor->setCanvasSize(ivec2(size, size));

        network->unlock();

        data = canvasProcessor->getImageLayerAsCodedBuffer(type);

        // remove the network...
        network->lock();
        for (size_t i = 0; i < processors.size(); i++) network->removeProcessor(processors[i]);
        network->unlock();

        wm->setVisibile(true);
        portInspector->setActive(false);
    }
    return data;
}

bool NetworkEditor::isModified() const { return modified_; }
void NetworkEditor::setModified(const bool modified) {
    if (modified != modified_) {
        modified_ = modified;
        for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend();
             ++it)
            static_cast<NetworkEditorObserver*>(*it)->onModifiedStatusChanged(modified);
    }
}

////////////////////////////////////////////////////////
//   LOAD AND GET SNAPSHOT FROM EXTERNAL NETWORK      //
////////////////////////////////////////////////////////

void NetworkEditor::addExternalNetwork(std::string fileName, std::string identifierPrefix,
                                       ivec2 pos, unsigned int networkEditorFlags,
                                       ivec2 canvasSize) {
    ProcessorNetwork* network = InviwoApplication::getPtr()->getProcessorNetwork();
    network->lock();

    IvwDeserializer xmlDeserializer(fileName);
    ProcessorNetwork* processorNetwork = new ProcessorNetwork();
    processorNetwork->deserialize(xmlDeserializer);

    std::vector<Processor*> processors = processorNetwork->getProcessors();

    for (size_t i = 0; i < processors.size(); i++) {
        std::string newIdentifier = identifierPrefix + "_" + processors[i]->getIdentifier();
        processors[i]->setIdentifier(newIdentifier);
        ProcessorMetaData* meta = processors[i]->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
        meta->setPosition(meta->getPosition() + pos);
        network->addProcessor(processors[i]);
    }

    std::vector<PortConnection*> connections = processorNetwork->getConnections();
    for (size_t i = 0; i < connections.size(); i++) {
        PortConnection* connection = connections[i];
        Outport* outport = connection->getOutport();
        Inport* inport = connection->getInport();
        // first remove the connection from the loaded network to avoid an already connected warning
        processorNetwork->removeConnection(outport, inport);
        network->addConnection(outport, inport);
    }

    std::vector<PropertyLink*> links = processorNetwork->getLinks();
    for (size_t i = 0; i < links.size(); i++) {
        network->addLink(links[i]->getSourceProperty(), links[i]->getDestinationProperty());
    }

    network->unlock();
}

std::vector<std::string> NetworkEditor::saveSnapshotsInExternalNetwork(
    std::string externalNetworkFile, std::string identifierPrefix) {
    // turnoff sound
    BoolProperty* soundProperty = dynamic_cast<BoolProperty*>(
        InviwoApplication::getPtr()->getSettingsByType<SystemSettings>()->getPropertyByIdentifier(
            "enableSound"));
    bool isSoundEnabled = soundProperty->get();

    if (isSoundEnabled) soundProperty->set(false);

    std::vector<std::string> canvasSnapShotFiles;
    std::string directory = filesystem::getFileDirectory(externalNetworkFile);
    std::string workSpaceName = filesystem::getFileNameWithExtension(externalNetworkFile);
    std::string newFileName = filesystem::replaceFileExtension(workSpaceName, "png");
    std::vector<Processor*> processors =
        InviwoApplication::getPtr()->getProcessorNetwork()->getProcessors();

    for (size_t i = 0; i < processors.size(); i++) {
        if (processors[i]->getIdentifier().find(identifierPrefix) != std::string::npos) {
            CanvasProcessor* canvasProcessor = dynamic_cast<CanvasProcessor*>(processors[i]);

            if (canvasProcessor) {
                std::string snapShotFilePath =
                    directory + "snapshot_" + canvasProcessor->getIdentifier() + newFileName;
                canvasSnapShotFiles.push_back(snapShotFilePath);
                canvasProcessor->saveImageLayer(snapShotFilePath);
            }
        }
    }

    if (isSoundEnabled) soundProperty->set(true);

    return canvasSnapShotFiles;
}

void NetworkEditor::removeExternalNetwork(std::string identifierPrefix) {
    InviwoApplication::getPtr()->getProcessorNetwork()->lock();
    std::vector<Processor*> processors =
        InviwoApplication::getPtr()->getProcessorNetwork()->getProcessors();

    for (size_t i = 0; i < processors.size(); i++) {
        if (processors[i]->getIdentifier().find(identifierPrefix) != std::string::npos)
            InviwoApplication::getPtr()->getProcessorNetwork()->removeProcessor(processors[i]);
    }

    InviwoApplication::getPtr()->getProcessorNetwork()->unlock();
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
    InviwoApplication::getPtr()->getProcessorNetwork()->setModified(true);
    InviwoApplication::getPtr()->getProcessorNetworkEvaluator()->requestEvaluate();
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
    ProcessorMap::const_iterator it = processorGraphicsItems_.find(key);

    if (it != processorGraphicsItems_.end()) {
        return it->second;
    } else {
        return NULL;
    }
}

ConnectionGraphicsItem* NetworkEditor::getConnectionGraphicsItem(PortConnection* key) const {
    ConnectionMap::const_iterator it = connectionGraphicsItems_.find(key);

    if (it != connectionGraphicsItems_.end()) {
        return it->second;
    } else {
        return NULL;
    }
}

LinkConnectionGraphicsItem* NetworkEditor::getLinkGraphicsItem(ProcessorPair key) const {
    LinkMap::const_iterator it = linkGraphicsItems_.find(key);

    if (it != linkGraphicsItems_.end()) {
        return it->second;
    } else {
        return NULL;
    }
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
    ProcessorNetwork* network = InviwoApplication::getPtr()->getProcessorNetwork();

    for (ProcessorMap::iterator it = processorGraphicsItems_.begin();
         it != processorGraphicsItems_.end(); ++it) {
        QPointF pos = it->second->pos();
        QPointF newpos = snapToGrid(pos);
        if (pos != newpos) it->second->setPos(newpos);
    }

    if (connectionCurve_) {
        // connection drag mode
        Outport* startPort = connectionCurve_->getOutportGraphicsItem()->getPort();

        delete connectionCurve_;
        connectionCurve_ = NULL;

        ProcessorInportGraphicsItem* endItem = getProcessorInportGraphicsItemAt(e->scenePos());
        if (endItem && endItem->getPort()->canConnectTo(startPort)) {
            Inport* endPort = endItem->getPort();

            if (endPort->isConnected()) {
                SingleInport* singleInport = dynamic_cast<SingleInport*>(endPort);
                if (singleInport) {
                    network->removeConnection(singleInport->getConnectedOutport(), endPort);
                }
            }
            network->addConnection(startPort, endPort);
        }
        e->accept();

    } else if (linkCurve_) {
        // link drag mode
        ProcessorGraphicsItem* start =
            linkCurve_->getSrcProcessorLinkGraphicsItem()->getProcessorGraphicsItem();
        Processor* startProcessor = start->getProcessor();

        delete linkCurve_;
        linkCurve_ = NULL;

        ProcessorGraphicsItem* endProcessorItem = getProcessorGraphicsItemAt(e->scenePos());
        if (endProcessorItem) {
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
    LinkConnectionGraphicsItem* linkGraphicsItem = getLinkGraphicsItemAt(e->scenePos());
    ProcessorGraphicsItem* p = getProcessorGraphicsItemAt(e->scenePos());
    if (p) {
        Processor* processor = p->getProcessor();
        if (processor && processor->hasProcessorWidget()) {
            if (processor->getProcessorWidget()->isVisible()) {
                processor->getProcessorWidget()->hide();
            } else {
                processor->getProcessorWidget()->show();
            }
        }
        e->accept();
    } else if (linkGraphicsItem) {
        showLinkDialog(linkGraphicsItem->getSrcProcessorGraphicsItem()->getProcessor(),
                    linkGraphicsItem->getDestProcessorGraphicsItem()->getProcessor());
        e->accept();
    } else {
        QGraphicsScene::mouseDoubleClickEvent(e);
    }
}

void NetworkEditor::keyPressEvent(QKeyEvent* keyEvent) {
    if (keyEvent->key() == Qt::Key_Delete) {
        keyEvent->accept();
        ProcessorNetwork* network = InviwoApplication::getPtr()->getProcessorNetwork();

        // delete selected graphics items

        QList<QGraphicsItem*> selectedGraphicsItems = selectedItems();
        for (int i = 0; i < selectedGraphicsItems.size(); i++) {
            // Remove Connection
            ConnectionGraphicsItem* connectionGraphicsItem =
                qgraphicsitem_cast<ConnectionGraphicsItem*>(selectedGraphicsItems[i]);
            if (connectionGraphicsItem) {
                removeConnection(connectionGraphicsItem);
            }
        }

        selectedGraphicsItems = selectedItems();
        for (int i = 0; i < selectedGraphicsItems.size(); i++) {
            // Remove Link
            LinkConnectionGraphicsItem* linkGraphicsItem =
                qgraphicsitem_cast<LinkConnectionGraphicsItem*>(selectedGraphicsItems[i]);
            if (linkGraphicsItem) {
                removeLink(linkGraphicsItem);
                continue;
            }
        }

        selectedGraphicsItems = selectedItems();
        for (int i = 0; i < selectedGraphicsItems.size(); i++) {
            // Remove Processor
            ProcessorGraphicsItem* processorGraphicsItem =
                qgraphicsitem_cast<ProcessorGraphicsItem*>(selectedGraphicsItems[i]);
            if (processorGraphicsItem && !processorGraphicsItem->isEditingProcessorName()) {
                network->removeAndDeleteProcessor(processorGraphicsItem->getProcessor());
                continue;
            }
        }
    } else {
        KeyboardEvent pressKeyEvent(EventConverterQt::getKeyButton(keyEvent),
                                    EventConverterQt::getModifier(keyEvent),
                                    KeyboardEvent::KEY_STATE_PRESS);

        QList<QGraphicsItem*> selectedGraphicsItems = selectedItems();
        ProcessorNetworkEvaluator* network =
            InviwoApplication::getPtr()->getProcessorNetworkEvaluator();

        for (int i = 0; i < selectedGraphicsItems.size(); i++) {
            ProcessorGraphicsItem* processorGraphicsItem =
                qgraphicsitem_cast<ProcessorGraphicsItem*>(selectedGraphicsItems[i]);
            if (processorGraphicsItem) {
                Processor* p = processorGraphicsItem->getProcessor();
                network->propagateInteractionEvent(p, &pressKeyEvent);
                if (pressKeyEvent.hasBeenUsed()) break;
            }
        }

        if (pressKeyEvent.hasBeenUsed()) {
            keyEvent->accept();
        } else {
            keyEvent->ignore();
            QGraphicsScene::keyPressEvent(keyEvent);
        }
    }
}


void NetworkEditor::keyReleaseEvent(QKeyEvent* keyEvent) {
    KeyboardEvent releaseKeyEvent(EventConverterQt::getKeyButton(keyEvent),
                                  EventConverterQt::getModifier(keyEvent),
                                  KeyboardEvent::KEY_STATE_RELEASE);

    QList<QGraphicsItem*> selectedGraphicsItems = selectedItems();
    ProcessorNetworkEvaluator* network =
        InviwoApplication::getPtr()->getProcessorNetworkEvaluator();

    for (int i = 0; i < selectedGraphicsItems.size(); i++) {
        ProcessorGraphicsItem* processorGraphicsItem =
            qgraphicsitem_cast<ProcessorGraphicsItem*>(selectedGraphicsItems[i]);
        if (processorGraphicsItem) {
            Processor* p = processorGraphicsItem->getProcessor();
            network->propagateInteractionEvent(p, &releaseKeyEvent);
            if (releaseKeyEvent.hasBeenUsed()) break;
        }
    }

    if (releaseKeyEvent.hasBeenUsed()) {
        keyEvent->accept();
    } else {
        keyEvent->ignore();
        QGraphicsScene::keyPressEvent(keyEvent);
    }
}

void NetworkEditor::contextMenuEvent(QGraphicsSceneContextMenuEvent* e) {
    QMenu menu;

    SignalMapperObject moRename;
    SignalMapperObject moDelete;
#if IVW_PROFILING
    SignalMapperObject moResetTM;
#endif
    SignalMapperObject moShowHide;

    SignalMapperObject moShowInspector;
    SignalMapperObject moDeleteLink;
    SignalMapperObject moEditLink;
    SignalMapperObject moDeleteConnection;

    QList<QGraphicsItem*> graphicsItems = items(e->scenePos());
    for (int i = 0; i < std::min(1, graphicsItems.size()); i++) {
        ProcessorOutportGraphicsItem* outport =
            qgraphicsitem_cast<ProcessorOutportGraphicsItem*>(graphicsItems[i]);
        if (outport) {
            QAction* showPortInsector = menu.addAction(tr("Port Inspector"));
            showPortInsector->setCheckable(true);
            if (portInspectors_.find(outport->getPort()) != portInspectors_.end()) {
                showPortInsector->setChecked(true);
            }
            moShowInspector.item_ = outport;
            connect(showPortInsector, SIGNAL(triggered()), &moShowInspector, SLOT(tiggerAction()));
            connect(&moShowInspector, SIGNAL(triggered(EditorGraphicsItem*)), this,
                    SLOT(contextMenuShowInspector(EditorGraphicsItem*)));
        }

        ProcessorInportGraphicsItem* inport =
            qgraphicsitem_cast<ProcessorInportGraphicsItem*>(graphicsItems[i]);
        if (inport) {
            QAction* showPortInsector = menu.addAction(tr("Port Inspector"));
            showPortInsector->setCheckable(true);
            if (portInspectors_.find(inport->getPort()->getConnectedOutport()) !=
                portInspectors_.end()) {
                showPortInsector->setChecked(true);
            }
            moShowInspector.item_ = inport;
            connect(showPortInsector, SIGNAL(triggered()), &moShowInspector, SLOT(tiggerAction()));
            connect(&moShowInspector, SIGNAL(triggered(EditorGraphicsItem*)), this,
                    SLOT(contextMenuShowInspector(EditorGraphicsItem*)));
        }

        ProcessorGraphicsItem* processor =
            qgraphicsitem_cast<ProcessorGraphicsItem*>(graphicsItems[i]);
        if (processor) {
            QAction* renameAction = menu.addAction(tr("Rename Processor"));
            QAction* deleteAction = menu.addAction(tr("Delete Processor"));

            moRename.item_ = processor;
            moDelete.item_ = processor;

            connect(renameAction, SIGNAL(triggered()), &moRename, SLOT(tiggerAction()));
            connect(&moRename, SIGNAL(triggered(EditorGraphicsItem*)), this,
                    SLOT(contextMenuRenameProcessor(EditorGraphicsItem*)));

            connect(deleteAction, SIGNAL(triggered()), &moDelete, SLOT(tiggerAction()));
            connect(&moDelete, SIGNAL(triggered(EditorGraphicsItem*)), this,
                    SLOT(contextMenuDeleteProcessor(EditorGraphicsItem*)));

#if IVW_PROFILING
            QAction* resetTimeMeasurementsAction = menu.addAction(tr("Reset Time Measurements"));
            moResetTM.item_ = processor;
            connect(resetTimeMeasurementsAction, SIGNAL(triggered()), &moResetTM, SLOT(tiggerAction()));
            connect(&moResetTM, SIGNAL(triggered(EditorGraphicsItem*)), this,
                SLOT(contextMenuResetTimeMeasurements(EditorGraphicsItem*)));
#endif

            if (processor->getProcessor()->hasProcessorWidget()) {
                QAction* showAction = menu.addAction(tr("Show Widget"));
                showAction->setCheckable(true);
                moShowHide.item_ = processor;
                connect(showAction, SIGNAL(triggered()), &moShowHide, SLOT(tiggerAction()));
                connect(&moShowHide, SIGNAL(triggered(EditorGraphicsItem*)), this,
                        SLOT(contextMenuShowHideWidget(EditorGraphicsItem*)));

                ProcessorWidget* processorWidget = processor->getProcessor()->getProcessorWidget();
                if (processorWidget) {
                    showAction->setChecked(processorWidget->isVisible());
                }
            }
        }

        ConnectionGraphicsItem* portconnection =
            qgraphicsitem_cast<ConnectionGraphicsItem*>(graphicsItems[i]);
        if (portconnection) {
            QAction* deleteConnection = menu.addAction(tr("Delete Connection"));
            moDeleteConnection.item_ = portconnection;
            connect(deleteConnection, SIGNAL(triggered()), &moDeleteConnection,
                    SLOT(tiggerAction()));
            connect(&moDeleteConnection, SIGNAL(triggered(EditorGraphicsItem*)), this,
                    SLOT(contextMenuDeleteConnection(EditorGraphicsItem*)));

            QAction* showPortInsector = menu.addAction(tr("Show Port Inspector"));
            showPortInsector->setCheckable(true);
            if (portInspectors_.find(portconnection->getOutportGraphicsItem()->getPort()) !=
                portInspectors_.end()) {
                showPortInsector->setChecked(true);
            }
            moShowInspector.item_ = portconnection->getOutportGraphicsItem();
            connect(showPortInsector, SIGNAL(triggered()), &moShowInspector, SLOT(tiggerAction()));
            connect(&moShowInspector, SIGNAL(triggered(EditorGraphicsItem*)), this,
                    SLOT(contextMenuShowInspector(EditorGraphicsItem*)));
        }

        LinkConnectionGraphicsItem* linkconnection =
            qgraphicsitem_cast<LinkConnectionGraphicsItem*>(graphicsItems[i]);
        if (linkconnection) {
            QAction* deleteLink = menu.addAction(tr("Delete Links"));
            moDeleteLink.item_ = linkconnection;
            connect(deleteLink, SIGNAL(triggered()), &moDeleteLink, SLOT(tiggerAction()));
            connect(&moDeleteLink, SIGNAL(triggered(EditorGraphicsItem*)), this,
                    SLOT(contextMenuDeleteLink(EditorGraphicsItem*)));

            QAction* editLink = menu.addAction(tr("Edit Links"));
            moEditLink.item_ = linkconnection;
            connect(editLink, SIGNAL(triggered()), &moEditLink, SLOT(tiggerAction()));
            connect(&moEditLink, SIGNAL(triggered(EditorGraphicsItem*)), this,
                    SLOT(contextMenuEditLink(EditorGraphicsItem*)));
        }
    }
    if (menu.actions().size() > 0) {
        menu.exec(QCursor::pos());
        e->accept();
    } else {
        QGraphicsScene::contextMenuEvent(e);
    }
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
            Processor* processor = static_cast<Processor*>(
                ProcessorFactory::getPtr()->create(className.toLocal8Bit().constData()));
            
            bool inputmatch = false;
            // try to find a match between connection outport and one of the new processors inports
            for (size_t i = 0; i < processor->getInports().size(); ++i) {
                if (processor->getInports().at(i)->canConnectTo(connectionItem->getOutport())) {
                    inputmatch = true;
                    break;
                }
            }

            bool outputmatch = false;
            // try to find a match between connection inport and one of the new processors outports
            for (size_t i = 0; i < processor->getOutports().size(); ++i) {
                if (connectionItem->getInport()->canConnectTo(processor->getOutports().at(i))) {
                    outputmatch = true;
                    break;
                }
            }
            if(inputmatch && outputmatch) {
                connectionItem->setBorderColor(Qt::green);
            } else {
                connectionItem->setBorderColor(Qt::red);
            }
            oldConnectionTarget_ = connectionItem;
            oldConnectionTarget_->setMidPoint(e->scenePos());
            delete processor;

        } else if (connectionItem) {  // move event on active connection
            oldConnectionTarget_->setMidPoint(e->scenePos());

        } else if (oldConnectionTarget_ && !connectionItem) {  //< Connection no longer targeted
            oldConnectionTarget_->resetBorderColors();
            oldConnectionTarget_->clearMidPoint();
            oldConnectionTarget_ = NULL;

        } else if (!connectionItem) {  // processor replacement
            ProcessorGraphicsItem* processorItem = getProcessorGraphicsItemAt(e->scenePos());

            if (processorItem && !oldProcessorTarget_) {  //< New processor found
                QString className;
                ProcessorDragObject::decode(e->mimeData(), className);
                processorItem->setSelected(true);
                oldProcessorTarget_ = processorItem;
            } else if (!processorItem && oldProcessorTarget_) {  // processor no longer targeted
                oldProcessorTarget_->setSelected(false);
                oldProcessorTarget_ = NULL;
            }
        }
    }
}

void NetworkEditor::dropEvent(QGraphicsSceneDragDropEvent* e) {
    if (ProcessorDragObject::canDecode(e->mimeData())) {
        QString className;
        ProcessorDragObject::decode(e->mimeData(), className);

        if (!className.isEmpty()) {
            e->setAccepted(true);
            e->acceptProposedAction();

            // create processor, add it to processor network, and generate it's widgets
            Processor* processor = static_cast<Processor*>(
                ProcessorFactory::getPtr()->create(className.toLocal8Bit().constData()));

            clearSelection();

            ProcessorMetaData* meta = processor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);

            if (oldProcessorTarget_) {
                meta->setPosition(
                    vec2(oldProcessorTarget_->scenePos().x(), oldProcessorTarget_->scenePos().y()));
            } else {
                QPointF spos = snapToGrid(e->scenePos());
                meta->setPosition(vec2(spos.x(), spos.y()));
            }

            InviwoApplication::getPtr()->getProcessorNetwork()->addProcessor(processor);

            InviwoApplication::getPtr()->getProcessorNetwork()->autoLinkProcessor(processor);
    
            if (oldConnectionTarget_) {
                placeProcessorOnConnection(processor, oldConnectionTarget_);
            } else if (oldProcessorTarget_) {
                placeProcessorOnProcessor(processor, oldProcessorTarget_->getProcessor());
            }

            // clear oldDragTarget
            oldConnectionTarget_ = NULL;
            oldProcessorTarget_ = NULL;
        }
    }
}

void NetworkEditor::placeProcessorOnConnection(Processor* processor,
                                               ConnectionGraphicsItem* connectionItem) {
    std::vector<Inport*> inports = processor->getInports();
    std::vector<Outport*> outports = processor->getOutports();
    Inport* connectionInport = connectionItem->getInport();
    Outport* connectionOutport = connectionItem->getOutport();

    InviwoApplication::getPtr()->getProcessorNetwork()->lock();

    Inport* inport = NULL;
    Outport* outport = NULL;

    for (size_t i = 0; i < inports.size(); ++i) {
        if (inports.at(i)->canConnectTo(connectionOutport)) {
            inport = inports.at(i);
            break;
        }
    }
    for (size_t i = 0; i < outports.size(); ++i) {
        if (connectionInport->canConnectTo(outports.at(i))) {
            outport = outports.at(i);
            break;
        }
    }

    if(inport && outport) {
        ProcessorNetwork* network = InviwoApplication::getPtr()->getProcessorNetwork();
        network->lock();
        
        // Remove old connection
        network->removeConnection(connectionOutport, connectionInport);
        
        // Add new Connections
        network->addConnection(connectionOutport, inport);
        network->addConnection(outport, connectionInport);
        
        network->unlock();
    }else{
        connectionItem->resetBorderColors();
        connectionItem->clearMidPoint();
    }

    InviwoApplication::getPtr()->getProcessorNetwork()->unlock();
}

void NetworkEditor::placeProcessorOnProcessor(Processor* newProcessor,
                                              Processor* oldProcessor) {

    ProcessorNetwork* network = InviwoApplication::getPtr()->getProcessorNetwork();

    const std::vector<Inport*>& inports = newProcessor->getInports();
    const std::vector<Outport*>& outports = newProcessor->getOutports();
    const std::vector<Inport*>& oldInports = oldProcessor->getInports();
    const std::vector<Outport*>& oldOutports = oldProcessor->getOutports();

    network->lock();

    std::vector<std::pair<Outport*, Inport*> > newConnections;

    for (size_t i = 0; i < std::min(inports.size(), oldInports.size()); ++i) {
        if (inports.at(i)->canConnectTo(oldInports.at(i)->getConnectedOutport())) {
            // MultiInports may have several connected outports
            std::vector<Outport*> previouslyConnectedOutports =
                oldInports.at(i)->getConnectedOutports();
            for (std::vector<Outport*>::iterator it = previouslyConnectedOutports.begin(),
                                                 itEnd = previouslyConnectedOutports.end();
                 it != itEnd; ++it) {
                // save new connection connectionOutportoldInport-processorInport
                newConnections.push_back(std::make_pair(*it, inports.at(i)));
            }
        }
    }

    for (size_t i = 0; i < std::min(outports.size(), oldOutports.size()); ++i) {
        std::vector<Inport*> previouslyConnectedInports = oldOutports.at(i)->getConnectedInports();

        for (size_t j = 0; j < previouslyConnectedInports.size(); ++j) {
            if (previouslyConnectedInports.at(j)->canConnectTo(outports.at(i))) {
                // save new connection processorOutport-connectionInport
                newConnections.push_back(
                    std::make_pair(outports.at(i), previouslyConnectedInports.at(j)));
            }
        }
    }
    
    
    // Copy over the value of old props to new ones if id and classname are equal.
    std::vector<Property*> newProps = newProcessor->getProperties();
    std::vector<Property*> oldProps = oldProcessor->getProperties();
    
    std::map<Property*, Property*> propertymap;
    
    for (std::vector<Property*>::iterator newit = newProps.begin(); newit != newProps.end(); ++newit) {
        for (std::vector<Property*>::iterator oldit = oldProps.begin(); oldit != oldProps.end(); ++oldit) {
            if ( (*newit)->getIdentifier() == (*oldit)->getIdentifier()
                && (*newit)->getClassIdentifier() == (*oldit)->getClassIdentifier()) {
                (*newit)->set(*oldit);
                propertymap[(*oldit)] = (*newit);
            }
        }
    }
    
    // Move propertylinks to the new processor
    std::vector<PropertyLink*> links = network->getLinks();
    std::map<Property*, Property*>::iterator match;
    
    for (std::vector<Property*>::iterator oldit = oldProps.begin(); oldit != oldProps.end(); ++oldit) {
        for (std::vector<PropertyLink*>::iterator linkit = links.begin(); linkit != links.end(); ++linkit) {
            if ( (*linkit)->getDestinationProperty() == (*oldit) ) {
                match = propertymap.find(*oldit);
                if( match != propertymap.end()) {
                    // add link from
                    Property* start = (*linkit)->getSourceProperty();
                    // to
                    Property* end = match->second;
                    
                    network->addLink(start, end);
                }
            }
            if ( (*linkit)->getSourceProperty() == (*oldit) ) {
                match = propertymap.find(*oldit);
                if( match != propertymap.end()) {
                    // add link from
                    Property* start = match->second;
                     //to
                    Property* end = (*linkit)->getDestinationProperty();
                    
                    network->addLink(start, end);
                }
            }
        }
    }
       
    // remove old processor
    network->removeProcessor(oldProcessor);

    // create all new connections
    for (size_t i = 0; i < newConnections.size(); ++i)
        network->addConnection(newConnections.at(i).first, newConnections.at(i).second);

    network->unlock();
}

///////////////////////////////
//   SERIALIZATION METHODS   //
///////////////////////////////
void NetworkEditor::clearNetwork() {

    //TODO move to ProecssorNetwork
    InviwoApplication::getPtr()->getProcessorNetwork()->lock();

    // We need to clear the portInspectors manually otherwise the pointers in the 
    // PortInspector networks won't be cleared
    PortInspectorMap portInspectors = portInspectors_;
    for (PortInspectorMap::iterator it = portInspectors.begin(); it != portInspectors.end(); ++it) {
        removePortInspector(it->first);
    }

    // Invalidate inports to alert processors that they should stop their calculations.
    std::vector<Processor*> processors =
        InviwoApplication::getPtr()->getProcessorNetwork()->getProcessors();
    for (size_t p = 0; p < processors.size(); p++) {
        std::vector<Inport*> inports = processors[p]->getInports();
        for (size_t i = 0; i < inports.size(); i++)
            inports[i]->invalidate(INVALID_OUTPUT);
    }

    ResourceManager::getPtr()->clearAllResources();

    for (size_t i = 0; i < processors.size(); i++) {
        InviwoApplication::getPtr()->getProcessorNetwork()->removeAndDeleteProcessor(processors[i]);
    }

    InviwoApplication::getPtr()->getProcessorNetwork()->unlock();
    setModified(true);
}

bool NetworkEditor::saveNetwork(std::string fileName) {
    try {
        IvwSerializer xmlSerializer(fileName);
        InviwoApplication::getPtr()->getProcessorNetwork()->serialize(xmlSerializer);
        InviwoApplication::getPtr()->getProcessorNetwork()->setModified(false);
        xmlSerializer.writeFile();
        filename_ = fileName;
        setModified(false);
    } catch (SerializationException& exception) {
        LogInfo("Unable to save network " + fileName + " due to " + exception.getMessage());
        return false;
    }
    return true;
}

bool NetworkEditor::saveNetwork(std::ostream stream) {
    try {
        IvwSerializer xmlSerializer(filename_);
        InviwoApplication::getPtr()->getProcessorNetwork()->serialize(xmlSerializer);
        InviwoApplication::getPtr()->getProcessorNetwork()->setModified(false);
        xmlSerializer.writeFile(stream);
        setModified(false);
    } catch (SerializationException& exception) {
        LogInfo("Unable to save network " + filename_ + " due to " + exception.getMessage());
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
    if (loaded) {
        for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend();
             ++it)
            static_cast<NetworkEditorObserver*>(*it)->onNetworkEditorFileChanged(fileName);
    }

    return loaded;
}

bool NetworkEditor::loadNetwork(std::istream& stream, const std::string& path) {
    // first we clean the current network
    clearNetwork();
    // then we lock the network that so no evaluations are triggered during the deserialization
    InviwoApplication::getPtr()->getProcessorNetwork()->lock();

    // then we deserialize processor network
    try {
        IvwDeserializer xmlDeserializer(stream, path);
        InviwoApplication::getPtr()->getProcessorNetwork()->deserialize(xmlDeserializer);
    } catch (const AbortException& exception) {
        LogInfo("Unable to load network " + path + " due to " + exception.getMessage());
        clearNetwork();
        InviwoApplication::getPtr()->getProcessorNetwork()->unlock();
        return false;
    } catch (const IgnoreException& exception) {
        LogInfo("Incomplete network loading " + path + " due to " + exception.getMessage());
    }

    propertyListWidget_->setUsageMode(InviwoApplication::getPtr()
                                      ->getSettingsByType<SystemSettings>()
                                      ->getApplicationUsageMode());

    InviwoApplication::getPtr()->getProcessorNetwork()->setModified(true);

    InviwoApplication::getPtr()->getProcessorNetwork()->unlock();

    setModified(false);
    filename_ = path;
    return true;
}

bool NetworkEditor::event(QEvent* e) {
   if (e->type() == PortInspectorEvent::type()) {
        e->accept();
        PortInspectorEvent* pie = static_cast<PortInspectorEvent*>(e);
        removePortInspector(pie->port_);
        return true;
    }
    return QGraphicsScene::event(e);
}

////////////////////////
//   HELPER METHODS   //
////////////////////////
QPointF NetworkEditor::snapToGrid(QPointF pos) {
    QPointF result;

    float ox = pos.x() > 0 ? 0.5 : -0.5;
    float oy = pos.y() > 0 ? 0.5 : -0.5;

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
    painter->setPen(QColor(153,153,153));

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
    for (ProcessorMap::iterator it = processorGraphicsItems_.begin();
         it != processorGraphicsItems_.end(); it++) {
        it->second->getStatusItem()->update();
    }
}

void NetworkEditor::resetAllTimeMeasurements() {
#if IVW_PROFILING
    // Update the status items
    for (ProcessorMap::iterator it = processorGraphicsItems_.begin();
        it != processorGraphicsItems_.end(); it++) {
            it->second->resetTimeMeasurements();
    }
#endif
}

// Manage various tooltips.
void NetworkEditor::helpEvent(QGraphicsSceneHelpEvent* e) {
    QList<QGraphicsItem*> graphicsItems = items(e->scenePos());
    for (int i = 0; i < graphicsItems.size(); ++i) {
        QGraphicsItem* item = graphicsItems[i];
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

            //case ProcessorLinkGraphicsItem::Type:
            //    qgraphicsitem_cast<ProcessorLinkGraphicsItem*>(item)->showToolTip(e);
            //    return;
            case ProcessorInportGraphicsItem::Type:
                qgraphicsitem_cast<ProcessorInportGraphicsItem*>(item)->showToolTip(e);
                return;
            case ProcessorOutportGraphicsItem::Type:
                qgraphicsitem_cast<ProcessorOutportGraphicsItem*>(item)->showToolTip(e);
                return;
            //case ProcessorStatusGraphicsItem::Type:
            //    qgraphicsitem_cast<ProcessorStatusGraphicsItem*>(item)->showToolTip(e);
            //    return;
            //case ProcessorProgressGraphicsItem::Type:
            //    qgraphicsitem_cast<ProcessorProgressGraphicsItem*>(item)->showToolTip(e);
            //    return;
        };
    }
    QGraphicsScene::helpEvent(e);
}

void NetworkEditor::contextMenuRenameProcessor(EditorGraphicsItem* item) {
    clearSelection();
    ProcessorGraphicsItem* p = qgraphicsitem_cast<ProcessorGraphicsItem*>(item);
    if (p) {
        p->setSelected(true);
        p->editProcessorName();
    }
}

void NetworkEditor::contextMenuShowInspector(EditorGraphicsItem* item) {   
    Outport* port = NULL;
    QPointF pos;
    ProcessorInportGraphicsItem* pin = qgraphicsitem_cast<ProcessorInportGraphicsItem*>(item);
    ProcessorOutportGraphicsItem* pout = qgraphicsitem_cast<ProcessorOutportGraphicsItem*>(item);
    if (pin) {
        pos = pin->mapPosToSceen(pin->rect().center());
        port = pin->getPort()->getConnectedOutport();
    } else if (pout) {
        pos = pout->mapPosToSceen(pout->rect().center());
        port = pout->getPort();
    } else {
        return;
    }
    
    if (portInspectors_.find(port)==portInspectors_.end()) {
        addPortInspector(port, pos);
    } else {
        removePortInspector(port);
    }
}

void NetworkEditor::contextMenuDeleteProcessor(EditorGraphicsItem* item) {
    ProcessorGraphicsItem* p = qgraphicsitem_cast<ProcessorGraphicsItem*>(item);
    if (p) {
        Processor* processor = p->getProcessor();
        InviwoApplication::getPtr()->getProcessorNetwork()->removeAndDeleteProcessor(processor);
    }
}

void NetworkEditor::contextMenuShowHideWidget(EditorGraphicsItem* item) {
    ProcessorGraphicsItem* p = qgraphicsitem_cast<ProcessorGraphicsItem*>(item);
    if (p) {
        Processor* processor = p->getProcessor();
        if (processor->getProcessorWidget()->isVisible()) {
            processor->getProcessorWidget()->hide();
        } else {
            processor->getProcessorWidget()->show();
        }
    }
}
void NetworkEditor::contextMenuDeleteConnection(EditorGraphicsItem* item) {
    ConnectionGraphicsItem* c = qgraphicsitem_cast<ConnectionGraphicsItem*>(item);
    if (c) {
        removeConnection(c);
    }
}
void NetworkEditor::contextMenuDeleteLink(EditorGraphicsItem* item) {
    LinkConnectionGraphicsItem* l = qgraphicsitem_cast<LinkConnectionGraphicsItem*>(item);
    if (l) {
        removeLink(l);
    }
}

void NetworkEditor::contextMenuEditLink(EditorGraphicsItem* item) {
    LinkConnectionGraphicsItem* l = qgraphicsitem_cast<LinkConnectionGraphicsItem*>(item);
    if (l) {
        showLinkDialog(l->getSrcProcessorGraphicsItem()->getProcessor(),
                       l->getDestProcessorGraphicsItem()->getProcessor());
    }
}

void NetworkEditor::contextMenuResetTimeMeasurements(EditorGraphicsItem* item) {
#if IVW_PROFILING
    ProcessorGraphicsItem* p = qgraphicsitem_cast<ProcessorGraphicsItem*>(item);
    p->resetTimeMeasurements();
#endif
}

void NetworkEditor::onProcessorNetworkDidAddProcessor(Processor* processor) {
    setModified(true);
    ProcessorMetaData* meta =
        processor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);

    addProcessorRepresentations(processor, QPointF(meta->getPosition().x, meta->getPosition().y),
                                meta->isVisible(), meta->isSelected());
}

void NetworkEditor::onProcessorNetworkWillRemoveProcessor(Processor* processor) {
    setModified(true);
    
    std::vector<Outport*> outports = processor->getOutports();
    for (std::vector<Outport*>::iterator it = outports.begin(); it != outports.end(); ++it) {
        removePortInspector(*it);
    }
    
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
    LinkConnectionGraphicsItem* link = getLinkGraphicsItem(p1, p2);
    if (!link) {
        addLinkGraphicsItem(p1, p2);
    }
}

void NetworkEditor::onProcessorNetworkDidRemoveLink(PropertyLink* propertyLink) {
    setModified(true);
    if (InviwoApplication::getPtr()
            ->getProcessorNetwork()
            ->getLinksBetweenProcessors(
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
QEvent::Type PortInspectorEvent::PORT_INSPECTOR_EVENT = QEvent::None;

}  // namespace
