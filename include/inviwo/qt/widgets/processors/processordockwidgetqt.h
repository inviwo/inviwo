/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_PROCESSORDOCKWIDGETQT_H
#define IVW_PROCESSORDOCKWIDGETQT_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processorobserver.h>
#include <inviwo/core/processors/processorobserver.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/qt/widgets/inviwodockwidget.h>
#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>

namespace inviwo {

/**
 * \class ProcessorDockWidgetQt
 * \brief Base class for Qt processor widget using InviwoDockWidget.
 *
 * This Qt widget provides the basic functionality and setup
 * for creating a processor widget based on InviwoDockWidget.
 * The content of the dock widget is set by calling setContent().
 *
 * \see InviwoDockWidget ProcessorWidget
 */
class IVW_QTWIDGETS_API ProcessorDockWidgetQt : public InviwoDockWidget,
                                                public ProcessorWidget,
                                                public ProcessorObserver {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>

public:
    ProcessorDockWidgetQt(Processor *p, const QString &title = QString("ProcessorDockWidgetQt"),
                          QWidget *parent = nullptr);
    virtual ~ProcessorDockWidgetQt() = default;

    // Override ProcessorWidget
    virtual void setVisible(bool visible) override;
    virtual void show() override;
    virtual void hide() override;
    virtual void setPosition(glm::ivec2 pos) override;
    virtual void setDimensions(ivec2 dimensions) override;

    // Override ProcessorObserver
    virtual void onProcessorIdentifierChange(Processor *) override;

protected:
    // Override QWidget events
    virtual void resizeEvent(QResizeEvent *) override;
    virtual void moveEvent(QMoveEvent *) override;
};

}  // namespace

#endif  // IVW_PROCESSORDOCKWIDGETQT_H
