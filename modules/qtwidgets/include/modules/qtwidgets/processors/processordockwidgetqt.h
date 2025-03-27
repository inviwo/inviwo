/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <inviwo/core/processors/processor.h>        // for Processor, Processor::NameDispatche...
#include <inviwo/core/processors/processorwidget.h>  // for ProcessorWidget
#include <inviwo/core/util/glmvec.h>                 // for ivec2
#include <modules/qtwidgets/inviwodockwidget.h>      // for InviwoDockWidget

#include <QObject>                  // for Q_OBJECT
#include <QString>                  // for QString
#include <glm/ext/vector_int2.hpp>  // for ivec2

class QHideEvent;
class QMoveEvent;
class QResizeEvent;
class QShowEvent;
class QWidget;

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
class IVW_MODULE_QTWIDGETS_API ProcessorDockWidgetQt : public InviwoDockWidget,
                                                       public ProcessorWidget {
    Q_OBJECT
public:
    ProcessorDockWidgetQt(Processor* p, const QString& title = QString("ProcessorDockWidgetQt"),
                          QWidget* parent = nullptr);
    virtual ~ProcessorDockWidgetQt() = default;

    // Override ProcessorWidget
    using InviwoDockWidget::setVisible;
    virtual void setPosition(glm::ivec2 pos) override;
    virtual void setDimensions(ivec2 dimensions) override;

protected:
    // Implement ProcessorWidget
    virtual void updateVisible(bool visible) override;
    virtual void updateDimensions(ivec2 size) override;
    virtual void updatePosition(ivec2 pos) override;
    virtual void updateFullScreen(bool) override;
    virtual void updateOnTop(bool) override;

    // Override QWidget events
    virtual void resizeEvent(QResizeEvent*) override;
    virtual void moveEvent(QMoveEvent*) override;
    virtual void showEvent(QShowEvent*) override;
    virtual void hideEvent(QHideEvent*) override;

    bool ignoreEvents_{false};
    bool resizeOngoing_{false};

    Processor::NameDispatcherHandle idChange_;
};

}  // namespace inviwo
