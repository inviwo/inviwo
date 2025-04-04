/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2025 Inviwo Foundation
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
#include <inviwo/core/interaction/events/eventpropagator.h>

#include <functional>   // for function
#include <memory>       // for unique_ptr
#include <string>       // for string
#include <string_view>  // for string_view
#include <vector>       // for vector

#include <QMainWindow>  // for QMainWindow

class QEvent;
class QMenu;
class QHideEvent;
class QMoveEvent;
class QResizeEvent;
class QShowEvent;

namespace inviwo {

class Canvas;
class CanvasQOpenGLWidget;
class PropertyListFrame;

class IVW_MODULE_OPENGLQT_API CanvasWithPropertiesProcessorWidgetQt : public CanvasProcessorWidget,
                                                                      public QMainWindow,
                                                                      public EventPropagator {
public:
    CanvasWithPropertiesProcessorWidgetQt(Processor* p);
    virtual ~CanvasWithPropertiesProcessorWidgetQt() = default;

    // CanvasProcessorWidget overrides
    virtual Canvas* getCanvas() const override;

    // ProcessorWidget overrides
    virtual void setVisible(bool visible) override;

    /**
     * Set the dimensions of the widget, i.e both canvas and properties, not just the canvas
     * This uses logical dimension, to be consistent with other widget
     */
    virtual void setDimensions(ivec2 dimensions) override;
    virtual void setPosition(ivec2 position) override;
    virtual void setFullScreen(bool fullScreen) override;
    virtual void setOnTop(bool onTop) override;

    // QMainWindow overrides
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void showEvent(QShowEvent*) override;
    virtual void hideEvent(QHideEvent*) override;
    virtual void moveEvent(QMoveEvent*) override;
    virtual void changeEvent(QEvent*) override;

    /**
     * A list of processor ids and/or property paths, separated by new lines to show in the
     * property list in the widget
     */
    void setProperties(std::string_view paths);

protected:
    using Super = QMainWindow;

    virtual void propagateEvent(Event* e, Outport* source) override;

    virtual void propagateResizeEvent() override;

    bool contextMenu(QMenu& menu);

    // ProcessorWidget overrides
    virtual void updateVisible(bool visible) override;
    virtual void updateDimensions(ivec2) override;
    virtual void updatePosition(ivec2) override;
    virtual void updateFullScreen(bool) override;
    virtual void updateOnTop(bool) override;

    std::unique_ptr<CanvasQOpenGLWidget, std::function<void(CanvasQOpenGLWidget*)>> canvas_;
    Processor::NameDispatcherHandle nameChange_;

    PropertyListFrame* frame_;
    std::vector<std::string> addedPaths_;
};

}  // namespace inviwo
