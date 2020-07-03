/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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
#include <modules/qtwidgets/inviwodockwidget.h>
#include <inviwo/core/properties/propertyvisibility.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <QEvent>
#include <warn/pop>
#include <unordered_map>

class QVBoxLayout;
class QScrollArea;

namespace inviwo {

class CollapsibleGroupBoxWidgetQt;
class Processor;
class InviwoApplication;

class IVW_MODULE_QTWIDGETS_API PropertyListFrame : public QWidget {
public:
    PropertyListFrame(QWidget* parent);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    void paintEvent(QPaintEvent*);
};

class IVW_MODULE_QTWIDGETS_API PropertyListEvent : public QEvent {
#include <warn/push>
#include <warn/ignore/all>
    Q_GADGET
#include <warn/pop>
public:
    enum class Action { Add = 0, Remove = 1 };
    PropertyListEvent(Action action, std::string processorId);
    static QEvent::Type type();
    Action action_;
    std::string processorId_;

private:
    static QEvent::Type PropertyListEventType;
};

class IVW_MODULE_QTWIDGETS_API PropertyListWidget : public InviwoDockWidget {
public:
    using WidgetMap = std::unordered_map<Processor*, CollapsibleGroupBoxWidgetQt*>;

    PropertyListWidget(QWidget* parent, InviwoApplication* app);
    virtual ~PropertyListWidget();

    void addProcessorProperties(Processor* processor);
    void removeProcessorProperties(Processor* processor);
    void removeAndDeleteProcessorProperties(Processor* processor);

    // Override QWidget
    virtual bool event(QEvent* e) override;

protected:
    WidgetMap widgetMap_;

private:
    CollapsibleGroupBoxWidgetQt* getPropertiesForProcessor(Processor* processor);
    CollapsibleGroupBoxWidgetQt* createPropertiesForProcessor(Processor* processor);

    InviwoApplication* app_;
    QVBoxLayout* listLayout_;
    QWidget* listWidget_;
    QScrollArea* scrollArea_;
};

}  // namespace inviwo
