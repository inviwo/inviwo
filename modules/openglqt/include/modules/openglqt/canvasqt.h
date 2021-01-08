/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2021 Inviwo Foundation
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

#pragma once

#include <modules/openglqt/openglqtmoduledefine.h>
#include <modules/openglqt/canvasqopenglwidget.h>
#include <inviwo/core/interaction/events/touchevent.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QHelpEvent>
#include <QGestureEvent>
#include <QTouchDevice>
#include <QTouchEvent>
#include <warn/pop>

namespace inviwo {

class CanvasQt : public CanvasQOpenGLWidget {
    friend class CanvasProcessorWidgetQt;

public:
    explicit CanvasQt(QWidget* parent, size2_t dim = size2_t(256, 256),
                      std::string_view name = "Canvas");
    virtual ~CanvasQt();

    virtual void render(std::shared_ptr<const Image> image, LayerType layerType = LayerType::Color,
                        size_t idx = 0) override;

    virtual std::unique_ptr<Canvas> createHiddenCanvas() override;

protected:
    virtual void setFullScreenInternal(bool fullscreen) override;
    virtual bool event(QEvent* e) override;

    void propagateEvent(Event* e);
    void propagateEvent(MouseInteractionEvent* e);
    bool showToolTip(QHelpEvent* e);

private:
    void doContextMenu(QMouseEvent* event);
    dvec2 normalPos(dvec2 pos) const;

    bool mapMousePressEvent(QMouseEvent* e);
    bool mapMouseDoubleClickEvent(QMouseEvent* e);
    bool mapMouseReleaseEvent(QMouseEvent* e);
    bool mapMouseMoveEvent(QMouseEvent* e);
    bool mapWheelEvent(QWheelEvent* e);
    bool mapKeyPressEvent(QKeyEvent* keyEvent);
    bool mapKeyReleaseEvent(QKeyEvent* keyEvent);
    bool mapTouchEvent(QTouchEvent* e);
    bool mapGestureEvent(QGestureEvent*);
    bool mapPanTriggered(QPanGesture*);
    bool mapPinchTriggered(QPinchGesture* e);

    //! Links QTouchDevice to inviwo::TouchDevice
    std::map<QTouchDevice*, TouchDevice> touchDevices_;
    //! Compare with next touch event to prevent duplicates
    std::vector<TouchPoint> prevTouchPoints_;
    Qt::GestureType lastType_{};
    int lastNumFingers_{0};
    vec2 screenPositionNormalized_{0};
    bool blockContextMenu_{false};

    std::string toolTipText_;
};

}  // namespace inviwo
