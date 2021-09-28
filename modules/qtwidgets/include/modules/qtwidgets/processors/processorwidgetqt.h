/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/processors/processor.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <warn/pop>

namespace inviwo {

class IVW_MODULE_QTWIDGETS_API ProcessorWidgetQt : public ProcessorWidget, public QWidget {
public:
    ProcessorWidgetQt(Processor* p);
    virtual ~ProcessorWidgetQt() = default;

    using QWidget::setVisible;
    virtual void setPosition(ivec2 pos) override;     // Override ProcessorWidget
    virtual void setDimensions(ivec2 dime) override;  // Override ProcessorWidget
    virtual void setFullScreen(bool fullScreen) override;
    virtual void setOnTop(bool onTop) override;

protected:
    virtual void updateVisible(bool visible) override;
    virtual void updateDimensions(ivec2) override;
    virtual void updatePosition(ivec2) override;
    virtual void updateFullScreen(bool) override;
    virtual void updateOnTop(bool) override;

    // Override QWidget events
    virtual void resizeEvent(QResizeEvent*) override;
    virtual void closeEvent(QCloseEvent*) override;
    virtual void showEvent(QShowEvent*) override;
    virtual void hideEvent(QHideEvent*) override;
    virtual void moveEvent(QMoveEvent*) override;

    bool ignoreEvents_{false};
    bool resizeOngoing_{false};

    Processor::NameDispatcherHandle nameChange_;
};

}  // namespace inviwo
