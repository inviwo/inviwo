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

#ifndef IVW_NETWORKEDITOR_H
#define IVW_NETWORKEDITOR_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QTimer>
#include <QThread>
#include <QPointF>
#include <QGraphicsSceneHelpEvent>

#include <inviwo/core/network/processornetworkevaluator.h>
#include <inviwo/core/util/singleton.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/processors/processorwidgetobserver.h>

namespace inviwo {

class Inport;
class Outport;
class PortInspector;
class EditorGraphicsItem;
class ProcessorGraphicsItem;
class ProcessorOutportGraphicsItem;
class ProcessorInportGraphicsItem;
class ProcessorLinkGraphicsItem;
class ConnectionGraphicsItem;
class ConnectionDragGraphicsItem;
class LinkConnectionGraphicsItem;
class LinkConnectionDragGraphicsItem;
class PropertyListWidget;
class LinkDialog;

class NetworkEditorObserver : public Observer {
public:
    virtual void onNetworkEditorFileChanged(const std::string& newFilename) = 0;
    virtual void onModifiedStatusChanged(const bool& newStatus) = 0;
};

/**
 * The NetworkEditor supports interactive editing of a ProcessorNetwork. Processors can be added
 * and removed, and their ports can be connected, while their properties can be linked.
 * - interactive editing, but if necessary use access functions
 * - graphical representation automatically managed
 * - inspector networks
 */
class IVW_QTEDITOR_API NetworkEditor : public QGraphicsScene,
                                       public Singleton<NetworkEditor>,
                                       public Observable<NetworkEditorObserver>,
                                       public ProcessorNetworkObserver {
    Q_OBJECT
public:
    NetworkEditor();
    virtual ~NetworkEditor();

    void clearNetwork();

    /**
     * Save network to stream.
     * Uses current file name for relative paths.
     *
     * @param std::ostream stream Stream to save data
     * @return bool true if successful, false otherwise.
     */
    bool saveNetwork(std::ostream stream);
    bool saveNetwork(std::string fileName);
    
    /**
     * Load network from a stream. The path will be used to calculate relative directories of data
     * (nothing will be stored in the path).
     * @param std::iostream& stream Stream with content that is to be deserialized.
     * @param std::string path A path that will be used to calculate location of data during
     * deserialization.
     */
    bool loadNetwork(std::istream& stream, const std::string& path);
    bool loadNetwork(std::string fileName);

    std::string getCurrentFilename() const { return filename_; }

    void addPropertyWidgets(Processor* processor);
    void removeAndDeletePropertyWidgets(Processor* processor);
    void removePropertyWidgets(Processor* processor);

    std::vector<std::string> getSnapshotsOfExternalNetwork(std::string fileName);

    bool isModified() const;
    void setModified(const bool modified = true);

    static QPointF snapToGrid(QPointF pos);

    // Called from ProcessorPortGraphicsItems mouse events.
    void initiateConnection(ProcessorOutportGraphicsItem* item);
    void releaseConnection(ProcessorInportGraphicsItem* item);

    // Called from ProcessorLinkGraphicsItems mouse event
    void initiateLink(ProcessorLinkGraphicsItem* item, QPointF pos);

    // Port inspectors
    bool addPortInspector(Outport* port, QPointF pos);
    void removePortInspector(Outport* port);
    std::vector<unsigned char>* renderPortInspectorImage(Port* port, std::string type);

    void updateLeds();

    void setPropertyListWidget(PropertyListWidget* widget);
    PropertyListWidget* getPropertyListWidget() const;

    // Overrides for ProcessorNetworkObserver 
    virtual void onProcessorNetworkChange();
    
    virtual void onProcessorNetworkDidAddProcessor(Processor* processor);
    virtual void onProcessorNetworkWillRemoveProcessor(Processor* processor);
    
    virtual void onProcessorNetworkDidAddConnection(PortConnection* connection);
    virtual void onProcessorNetworkWillRemoveConnection(PortConnection* connection);
    
    virtual void onProcessorNetworkDidAddLink(PropertyLink* propertyLink);
    virtual void onProcessorNetworkDidRemoveLink(PropertyLink* propertyLink);
    

public slots:
    void contextMenuRenameProcessor(EditorGraphicsItem*);
    void contextMenuShowInspector(EditorGraphicsItem*);
    void contextMenuDeleteProcessor(EditorGraphicsItem*);
    void contextMenuShowHideWidget(EditorGraphicsItem*);
    void contextMenuDeleteConnection(EditorGraphicsItem*);
    void contextMenuDeleteLink(EditorGraphicsItem*);
    void contextMenuEditLink(EditorGraphicsItem*);
    void contextMenuResetTimeMeasurements(EditorGraphicsItem*);

    void resetAllTimeMeasurements();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* e);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* e);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e);

    void keyPressEvent(QKeyEvent* keyEvent);
    void keyReleaseEvent(QKeyEvent* keyEvent);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* e);

    void dragEnterEvent(QGraphicsSceneDragDropEvent* de);
    void dragMoveEvent(QGraphicsSceneDragDropEvent* de);
    void dropEvent(QGraphicsSceneDragDropEvent* de);

    void placeProcessorOnConnection(Processor* processorItem,
                                    ConnectionGraphicsItem* connectionItem);
    void placeProcessorOnProcessor(Processor* processorItem,
                                   Processor* oldProcessorItem);

    // Override for tooltips
    void helpEvent(QGraphicsSceneHelpEvent* helpEvent);
    // Override for custom events
    virtual bool event(QEvent* e);
    

private:
    enum NetworkEditorFlags { None = 0, CanvasHidden = 1, UseOriginalCanvasSize = 1 << 2 };

    friend class ProcessorGraphicsItem;
    friend class ConnectionGraphicsItem;

    // Processors
    ProcessorGraphicsItem* addProcessorRepresentations(Processor* processor, QPointF pos,
                                                       bool showProcessor = true,
                                                       bool selectProcessor = true);
    void removeProcessorRepresentations(Processor* processor);
    ProcessorGraphicsItem* addProcessorGraphicsItem(Processor* processor, QPointF pos,
                                                    bool visible = true, bool selected = true);
    void removeProcessorGraphicsItem(Processor* processor);

    // Connections
    void removeConnection(ConnectionGraphicsItem* connectionGraphicsItem);
    ConnectionGraphicsItem* addConnectionGraphicsItem(PortConnection* connection);
    void removeConnectionGraphicsItem(PortConnection* connection);

    // Links
    void removeLink(LinkConnectionGraphicsItem* linkGraphicsItem);
    LinkConnectionGraphicsItem* addLinkGraphicsItem(Processor* processor1, Processor* processor2);
    void removeLinkGraphicsItem(LinkConnectionGraphicsItem* linkGraphicsItem);
    void showLinkDialog(Processor* processor1, Processor* processor2);

    ProcessorGraphicsItem* getProcessorGraphicsItem(Processor* key) const;
    ConnectionGraphicsItem* getConnectionGraphicsItem(PortConnection* key) const;
    LinkConnectionGraphicsItem* getLinkGraphicsItem(ProcessorPair key) const;
    LinkConnectionGraphicsItem* getLinkGraphicsItem(Processor* processor1,
                                                    Processor* processor2) const;

    // Get QGraphicsItems
    template <typename T>
    T* getGraphicsItemAt(const QPointF pos) const;
    ProcessorGraphicsItem* getProcessorGraphicsItemAt(const QPointF pos) const;
    ProcessorInportGraphicsItem* getProcessorInportGraphicsItemAt(const QPointF pos) const;
    ConnectionGraphicsItem* getConnectionGraphicsItemAt(const QPointF pos) const;
    LinkConnectionGraphicsItem* getLinkGraphicsItemAt(const QPointF pos) const;

    void addExternalNetwork(std::string fileName, std::string processorPrefix, ivec2 pos,
                            unsigned int networkEditorFlags = NetworkEditor::None,
                            ivec2 canvasSize = ivec2(128));
    void removeExternalNetwork(std::string identifierPrefix);
    std::vector<std::string> saveSnapshotsInExternalNetwork(std::string externalNetworkFile,
                                                            std::string identifierPrefix);

    void drawBackground(QPainter* painter, const QRectF& rect);

    typedef std::map<Processor*, ProcessorGraphicsItem*> ProcessorMap;
    typedef std::map<PortConnection*, ConnectionGraphicsItem*> ConnectionMap;
    typedef std::map<ProcessorPair, LinkConnectionGraphicsItem*> LinkMap;
    typedef std::map<Outport*, PortInspector*> PortInspectorMap;

    ProcessorMap processorGraphicsItems_;
    ConnectionMap connectionGraphicsItems_;
    LinkMap linkGraphicsItems_;
    PortInspectorMap portInspectors_;
    
    // Drag n drop state
    ConnectionGraphicsItem* oldConnectionTarget_;
    ProcessorGraphicsItem* oldProcessorTarget_;
    
    // Connection and link state
    ConnectionDragGraphicsItem* connectionCurve_;
    LinkConnectionDragGraphicsItem* linkCurve_;

    PropertyListWidget* propertyListWidget_;

    static const int GRID_SPACING;
    std::string filename_;
    bool modified_;
};


template <typename T>
T* inviwo::NetworkEditor::getGraphicsItemAt(const QPointF pos) const {
    QList<QGraphicsItem*> graphicsItems = items(pos);
    for (int i = 0; i < graphicsItems.size(); i++) {
        T* item = qgraphicsitem_cast<T*>(graphicsItems[i]);

        if (item) return item;
    }
    return nullptr;
}

class IVW_QTEDITOR_API PortInspectorObserver : public ProcessorWidgetObserver {
public:
    PortInspectorObserver(NetworkEditor* editor, Outport* port)
        : ProcessorWidgetObserver(), editor_(editor), port_(port) {}
    virtual void onProcessorWidgetHide(ProcessorWidget* widget);
    NetworkEditor* editor_;
    Outport* port_;
};

class IVW_QTEDITOR_API PortInspectorEvent : public QEvent {
    Q_GADGET
public:
    PortInspectorEvent(Outport* port) : QEvent(PORT_INSPECTOR_EVENT), port_(port) {}

    static QEvent::Type type() {
        if (PORT_INSPECTOR_EVENT == QEvent::None) {
            PORT_INSPECTOR_EVENT = static_cast<QEvent::Type>(QEvent::registerEventType());
        }
        return PORT_INSPECTOR_EVENT;
    }

    Outport* port_;

private:
    static QEvent::Type PORT_INSPECTOR_EVENT;
};

class SignalMapperObject : public QObject {
    Q_OBJECT
public:
    SignalMapperObject() : QObject(), item_(nullptr) {}

public slots:
    void tiggerAction() { emit(triggered(item_)); }

signals:
    void triggered(EditorGraphicsItem*);

public:
    EditorGraphicsItem* item_;
};

}  // namespace

#endif  // IVW_NETWORKEDITOR_H
