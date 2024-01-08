/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <modules/openglqt/openglqtmoduledefine.h>  // for IVW_MODULE_OPENGLQT_API

#include <inviwo/core/processors/canvasprocessorwidget.h>  // for CanvasProcessorWidget
#include <inviwo/core/processors/processor.h>              // for Processor, Processor::NameDisp...
#include <inviwo/core/util/glmvec.h>                       // for ivec2, size2_t

#include <functional>  // for function
#include <memory>      // for unique_ptr

#include <QIcon>     // for QIcon
#include <QLocale>   // for QLocale
#include <QPixmap>   // for QPixmap
#include <QVariant>  // for QVariant
#include <QWidget>   // for QWidget

class QHideEvent;
class QMenu;
class QMoveEvent;
class QResizeEvent;
class QShowEvent;

namespace inviwo {

class Canvas;
class CanvasQOpenGLWidget;

class IVW_MODULE_OPENGLQT_API CanvasProcessorWidgetQt : public CanvasProcessorWidget,
                                                        public QWidget {
public:
    CanvasProcessorWidgetQt(Processor* p);
    virtual ~CanvasProcessorWidgetQt();

    // Override ProcessorWidget
    virtual void setVisible(bool visible) override;
    virtual void setPosition(ivec2 pos) override;

    /**
     * Sets the physical (pixel) dimensions of the widget and the contained canvas
     * Note: QWidget::resize() uses logical dimensions. We use the physical dimensions and not the
     * logical ones here since there dimensions traditionally have corresponded to the texture
     * dimensions of the canvas, this way we keep that behavior even also for retina systems.
     */
    virtual void setDimensions(ivec2 dimensions) override;
    virtual void setFullScreen(bool fullScreen) override;
    virtual void setOnTop(bool onTop) override;

    virtual Canvas* getCanvas() const override;

protected:
    /**
     * Propagated an event with the physical (pixel) dimensions of the canvas
     * Note: QWidget::size() will return logical dimensions
     */
    virtual void propagateResizeEvent() override;
    bool contextMenu(QMenu& menu);

    virtual void updateVisible(bool visible) override;
    virtual void updateDimensions(ivec2) override;
    virtual void updatePosition(ivec2) override;
    virtual void updateFullScreen(bool) override;
    virtual void updateOnTop(bool) override;

    // Override QWidget events
    virtual void resizeEvent(QResizeEvent*) override;
    virtual void showEvent(QShowEvent*) override;
    virtual void hideEvent(QHideEvent*) override;
    virtual void moveEvent(QMoveEvent*) override;

private:
    using Super = QWidget;
    std::unique_ptr<CanvasQOpenGLWidget, std::function<void(CanvasQOpenGLWidget*)>> canvas_;

    bool ignoreEvents_{false};
    bool resizeOngoing_{false};

    size2_t canvasDimensions_{0};
    Processor::NameDispatcherHandle nameChange_;
};

}  // namespace inviwo
