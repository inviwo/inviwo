/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <inviwo/qt/editor/networkeditorobserver.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/network/processornetworkobserver.h>
#include <inviwo/core/network/portconnection.h>
#include <inviwo/core/processors/processorpair.h>
#include <inviwo/core/processors/processorwidgetobserver.h>
#include <inviwo/core/interaction/events/keyboardevent.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QTimer>
#include <QThread>
#include <QPointF>
#include <QGraphicsSceneHelpEvent>
#include <QMimeData>
#include <warn/pop>

namespace inviwo {

class Inport;
class Outport;
class PortInspector;
class ProcessorNetwork;
class EditorGraphicsItem;
class ProcessorGraphicsItem;
class ProcessorOutportGraphicsItem;
class ProcessorInportGraphicsItem;
class ProcessorLinkGraphicsItem;
class ConnectionGraphicsItem;
class ConnectionDragGraphicsItem;
class LinkConnectionGraphicsItem;
class LinkConnectionDragGraphicsItem;
class LinkDialog;
class InviwoMainWindow;
class Image;
class MenuItem;
class ProcessorDragHelper;
class ConnectionDragHelper;
class LinkDragHelper;
class TextLabelOverlay;

/**
 * The NetworkEditor supports interactive editing of a ProcessorNetwork. Processors can be added
 * and removed, and their ports can be connected, while their properties can be linked.
 * - interactive editing, but if necessary use access functions
 * - graphical representation automatically managed
 * - inspector networks
 */
class IVW_QTEDITOR_API NetworkEditor : public QGraphicsScene,
                                       public Observable<NetworkEditorObserver>,
                                       public ProcessorNetworkObserver {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    NetworkEditor(InviwoMainWindow* mainwindow);
    virtual ~NetworkEditor() = default;

    QByteArray copy() const;
    QByteArray cut();
    void paste(QByteArray data);
    void append(std::istream& is, const std::string& refPath = "");
    void selectAll();
    void deleteSelection();

    const std::string& getCurrentFilename() const;
    ProcessorNetwork* getNetwork() const;
    InviwoMainWindow* getMainWindow() const;
    TextLabelOverlay& getOverlay() const;

    void addPropertyWidgets(Processor* processor);
    void removeAndDeletePropertyWidgets(Processor* processor);
    void removePropertyWidgets(Processor* processor);
    void showProcessorHelp(const std::string& classIdentifier, bool raise = false);

    bool isModified() const;
    void setModified(const bool modified = true);

    static QPointF snapToGrid(QPointF pos);

    // Called from ProcessorPortGraphicsItems mouse events.
    void initiateConnection(ProcessorOutportGraphicsItem* item);
    void releaseConnection(ProcessorInportGraphicsItem* item);

    // Called from ProcessorLinkGraphicsItems mouse event
    void initiateLink(ProcessorLinkGraphicsItem* item, QPointF pos);

    // Port inspectors
    std::shared_ptr<const Image> renderPortInspectorImage(Outport* port);

    ProcessorGraphicsItem* getProcessorGraphicsItem(Processor* key) const;
    ConnectionGraphicsItem* getConnectionGraphicsItem(const PortConnection& key) const;
    LinkConnectionGraphicsItem* getLinkGraphicsItem(const ProcessorPair& key) const;
    LinkConnectionGraphicsItem* getLinkGraphicsItem(Processor* processor1,
                                                    Processor* processor2) const;
    ProcessorGraphicsItem* getProcessorGraphicsItemAt(const QPointF pos) const;
    ProcessorInportGraphicsItem* getProcessorInportGraphicsItemAt(const QPointF pos) const;
    ConnectionGraphicsItem* getConnectionGraphicsItemAt(const QPointF pos) const;
    LinkConnectionGraphicsItem* getLinkGraphicsItemAt(const QPointF pos) const;

    void setBackgroundVisible(bool visible);
    bool isBackgroundVisible() const;

    void updateSceneSize();
    QRectF getProcessorsBoundingRect() const;

    static std::string getMimeTag();
    void resetAllTimeMeasurements();

    void showLinkDialog(Processor* processor1, Processor* processor2);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) override;

    virtual void keyPressEvent(QKeyEvent* keyEvent) override;

    void progagateEventToSelecedProcessors(KeyboardEvent& pressKeyEvent);

    virtual void keyReleaseEvent(QKeyEvent* keyEvent) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* e) override;

    // Override for tooltips
    virtual void helpEvent(QGraphicsSceneHelpEvent* helpEvent) override;

    // override these event drag functions since QGraphicsScene will set the drop action of the
    // event to Qt::IgnoreAction if the event is not accepted by the scene
    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent*) override {}
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent*) override {}
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent*) override {}

private:
    friend class ProcessorGraphicsItem;
    friend class ConnectionGraphicsItem;

    class IVW_QTEDITOR_API AdjustSceneToChangesBlocker {
    public:
        AdjustSceneToChangesBlocker(NetworkEditor& network);
        AdjustSceneToChangesBlocker() = delete;
        AdjustSceneToChangesBlocker(const AdjustSceneToChangesBlocker&) = delete;
        AdjustSceneToChangesBlocker(AdjustSceneToChangesBlocker&&) = delete;
        AdjustSceneToChangesBlocker& operator=(AdjustSceneToChangesBlocker) = delete;
        ~AdjustSceneToChangesBlocker();

    private:
        NetworkEditor& network_;
    };

    // Overrides for ProcessorNetworkObserver
    virtual void onProcessorNetworkChange() override;

    virtual void onProcessorNetworkDidAddProcessor(Processor* processor) override;
    virtual void onProcessorNetworkWillRemoveProcessor(Processor* processor) override;

    virtual void onProcessorNetworkDidAddConnection(const PortConnection& connection) override;
    virtual void onProcessorNetworkWillRemoveConnection(const PortConnection& connection) override;

    virtual void onProcessorNetworkDidAddLink(const PropertyLink& propertyLink) override;
    virtual void onProcessorNetworkDidRemoveLink(const PropertyLink& propertyLink) override;

    // Processors
    ProcessorGraphicsItem* addProcessorRepresentations(Processor* processor);
    void removeProcessorRepresentations(Processor* processor);
    ProcessorGraphicsItem* addProcessorGraphicsItem(Processor* processor);
    void removeProcessorGraphicsItem(Processor* processor);

    // Connections
    void removeConnection(ConnectionGraphicsItem* connectionGraphicsItem);
    ConnectionGraphicsItem* addConnectionGraphicsItem(const PortConnection& connection);
    void removeConnectionGraphicsItem(const PortConnection& connection);

    // Links
    void removeLink(LinkConnectionGraphicsItem* linkGraphicsItem);
    LinkConnectionGraphicsItem* addLinkGraphicsItem(Processor* processor1, Processor* processor2);
    void removeLinkGraphicsItem(LinkConnectionGraphicsItem* linkGraphicsItem);

    // Get QGraphicsItems
    template <typename T>
    T* getGraphicsItemAt(const QPointF pos) const;

    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
    virtual void drawForeground(QPainter* painter, const QRectF& rect) override;

    void deleteItems(QList<QGraphicsItem*> items);

    using ProcessorMap = std::map<Processor*, ProcessorGraphicsItem*>;
    using ConnectionMap = std::map<PortConnection, ConnectionGraphicsItem*>;
    using LinkMap = std::map<ProcessorPair, LinkConnectionGraphicsItem*>;

    // Drag n drop state
    ProcessorDragHelper* processorDragHelper_;
    LinkDragHelper* linkDragHelper_;
    ConnectionDragHelper* connectionDragHelper_;
    ProcessorGraphicsItem* processorItem_;

    ProcessorMap processorGraphicsItems_;
    ConnectionMap connectionGraphicsItems_;
    LinkMap linkGraphicsItems_;

    QList<QGraphicsItem*> clickedOnItems_;
    std::pair<bool, ivec2> clickedPosition_ = {false, ivec2{0, 0}};
    mutable std::pair<bool, ivec2> pastePos_ = {false, ivec2{0, 0}};

    InviwoMainWindow* mainwindow_;
    ProcessorNetwork* network_;
    static const int gridSpacing_;
    std::string filename_;
    bool modified_;
    bool backgroundVisible_;

    bool adjustSceneToChange_;
};

template <typename T>
T* inviwo::NetworkEditor::getGraphicsItemAt(const QPointF pos) const {
    QList<QGraphicsItem*> graphicsItems = items(pos);
    for (int i = 0; i < graphicsItems.size(); i++) {
        if (auto item = qgraphicsitem_cast<T*>(graphicsItems[i])) return item;
    }
    return nullptr;
}

}  // namespace inviwo

#endif  // IVW_NETWORKEDITOR_H
