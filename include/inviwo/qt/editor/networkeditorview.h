/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#ifndef IVW_NETWORKEDITORVIEW_H
#define IVW_NETWORKEDITORVIEW_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/core/network/workspacemanager.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsView>
#include <QImage>
#include <warn/pop>

class QDropEvent;
class QDragEnterEvent;

namespace inviwo {

class InviwoMainWindow;
class MenuItem;
class NetworkSearch;
class TextLabelOverlay;

class IVW_QTEDITOR_API NetworkEditorView : public QGraphicsView, public NetworkEditorObserver {
public:
    NetworkEditorView(NetworkEditor* networkEditor, InviwoMainWindow* parent = nullptr);
    ~NetworkEditorView();

    void hideNetwork(bool);
    void fitNetwork();
    virtual void onNetworkEditorFileChanged(const std::string& newFilename) override;

    void exportViewToFile(const QString& filename, bool entireScene, bool backgroundVisible);
    QImage exportViewToImage(bool entireScene, bool backgroundVisible, QSize size = QSize());

    TextLabelOverlay& getOverlay() const;
    NetworkSearch& getNetworkSearch() const;

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent* e) override;
    virtual void resizeEvent(QResizeEvent* re) override;
    virtual void wheelEvent(QWheelEvent* e) override;

    virtual void keyPressEvent(QKeyEvent* keyEvent) override;
    virtual void keyReleaseEvent(QKeyEvent* keyEvent) override;
    virtual void focusOutEvent(QFocusEvent*) override;

private:
    void zoom(double dz);
    virtual void onSceneSizeChanged() override;

    InviwoMainWindow* mainwindow_;
    NetworkEditor* editor_;
    NetworkSearch* search_;
    TextLabelOverlay* overlay_;

    ivec2 scrollPos_;
    WorkspaceManager::DeserializationHandle loadHandle_;
    WorkspaceManager::ClearHandle clearHandle_;
    std::shared_ptr<MenuItem> editActionsHandle_;
};

}  // namespace inviwo

#endif  // IVW_NETWORKEDITORVIEW_H
