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
#include <modules/opengl/canvasgl.h>
#include <inviwo/core/interaction/events/eventpropagator.h>

#define QT_NO_OPENGL_ES_2
#define GLEXT_64_TYPES_DEFINED

#include <warn/push>
#include <warn/ignore/all>
#include <QOpenGLWidget>
#include <warn/pop>

class QResizeEvent;
class QHelpEvent;
class QPointingDevice;
class QTouchDevice;

namespace inviwo {

class IVW_MODULE_OPENGLQT_API CanvasQOpenGLWidget : public QOpenGLWidget,
                                                    public CanvasGL,
                                                    public EventPropagator {
    friend class CanvasProcessorWidgetQt;

public:
    explicit CanvasQOpenGLWidget(QWidget* parent, std::string_view name);
    virtual ~CanvasQOpenGLWidget();

    virtual void activate() override;
    virtual void glSwapBuffers() override;
    virtual void update() override;

    virtual void render(std::shared_ptr<const Image> image, LayerType layerType = LayerType::Color,
                        size_t idx = 0) override;
    
    virtual ContextID activeContext() const override;
    virtual ContextID contextId() const override;
    virtual std::unique_ptr<Canvas> createHiddenCanvas() override;
    
    virtual size2_t getCanvasDimensions() const override;
    
protected:
    // inviwo::Canvas override
    virtual void releaseContext() override;
    virtual void propagateEvent(Event* e, Outport* source) override;
    
    // QOpenGLWidget overrides
    virtual void initializeGL() override;
    virtual void paintGL() override;
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    void doContextMenu(QMouseEvent* event);

    std::string name_;

};

}  // namespace inviwo
