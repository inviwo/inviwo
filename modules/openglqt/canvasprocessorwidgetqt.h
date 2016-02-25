/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_CANVASPROCESSORWIDGETQT_H
#define IVW_CANVASPROCESSORWIDGETQT_H

#include <modules/openglqt/openglqtmoduledefine.h>
#include <inviwo/core/processors/canvasprocessorwidget.h>
#include <inviwo/core/processors/processorobserver.h>
#include <modules/openglqt/canvasqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <warn/pop>

namespace inviwo {

class CanvasProcessor;

class IVW_MODULE_OPENGLQT_API CanvasProcessorWidgetQt : public QWidget,
                                                        public CanvasProcessorWidget,
                                                        public ProcessorObserver {
    
    #include <warn/push>
    #include <warn/ignore/all>
    Q_OBJECT
    #include <warn/pop>
public:
    CanvasProcessorWidgetQt(Processor* p);
    virtual ~CanvasProcessorWidgetQt();

    // Override ProcessorWidget
    virtual void setVisible(bool visible) override;
    virtual void show() override;
    virtual void hide() override;
    virtual void setPosition(glm::ivec2 pos) override; 
    virtual void setDimensions(ivec2 dimensions) override;

    virtual Canvas* getCanvas() const override;

    // Override ProcessorObserver
    virtual void onProcessorIdentifierChange(Processor*) override;

protected:
    // Override QWidget events
    virtual void resizeEvent(QResizeEvent*) override;
    virtual void closeEvent(QCloseEvent*) override;
    virtual void showEvent(QShowEvent*) override;
    virtual void hideEvent(QHideEvent*) override;
    virtual void moveEvent(QMoveEvent*) override;

private:
    using canvas_ptr = std::unique_ptr<CanvasQt, std::function<void(CanvasQt*)>>;
    canvas_ptr canvas_;
};

}  // namespace

#endif  // IVW_CANVASPROCESSORWIDGETQT_H
