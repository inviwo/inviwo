/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_PROPERTYLISTWIDGET_H
#define IVW_PROPERTYLISTWIDGET_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/qt/widgets/inviwodockwidget.h>
#include <inviwo/core/properties/propertyvisibility.h>
#include <QWidget>
#include <QEvent>
#include <map>

class QVBoxLayout;
class QScrollArea;

namespace inviwo {

class CollapsibleGroupBoxWidgetQt;
class Processor;

class IVW_QTWIDGETS_API PropertyListFrame : public QWidget {
public:
    PropertyListFrame(QWidget* parent);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    void paintEvent(QPaintEvent*);
};

class IVW_QTWIDGETS_API PropertyListEvent : public QEvent {
    Q_GADGET
public:
    enum Action { ADD = 0, REMOVE = 1};

    PropertyListEvent(Action action, Processor* processor)
        : QEvent(PROPERY_LIST_EVENT), action_(action), processorId_(""), processor_(processor) {}

    PropertyListEvent(Action action, std::string processorId)
        : QEvent(PROPERY_LIST_EVENT), action_(action), processorId_(processorId), processor_(NULL) {}

    static QEvent::Type type() {
        if (PROPERY_LIST_EVENT == QEvent::None) {
            PROPERY_LIST_EVENT = static_cast<QEvent::Type>(QEvent::registerEventType());
        }
        return PROPERY_LIST_EVENT;
    }

    Action action_;
    std::string processorId_;
    Processor* processor_;

private:
    static QEvent::Type PROPERY_LIST_EVENT;
};

class IVW_QTWIDGETS_API PropertyListWidget : public InviwoDockWidget {
    Q_OBJECT
public:
    typedef std::map<Processor*, CollapsibleGroupBoxWidgetQt*> WidgetMap;

    PropertyListWidget(QWidget* parent);
    ~PropertyListWidget();

    void addProcessorProperties(Processor* processor);
    void removeProcessorProperties(Processor* processor);
    void removeAndDeleteProcessorProperties(Processor* processor);

    // Override QWidget
    virtual bool event(QEvent* e);

public slots:
    void setUsageMode(UsageMode viewMode);

protected:
    mutable WidgetMap widgetMap_;

private:
    CollapsibleGroupBoxWidgetQt* getPropertiesForProcessor(Processor* processor);
    CollapsibleGroupBoxWidgetQt* createPropertiesForProcessor(Processor* processor);

    QVBoxLayout* listLayout_;
    WidgetMap devWidgets_;
    QWidget* listWidget_;
    QScrollArea* scrollArea_;
};

}  // namespace

#endif  // IVW_PROPERTYLISTWIDGET_H