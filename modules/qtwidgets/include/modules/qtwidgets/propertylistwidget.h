/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2022 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>                 // for IVW_MODULE_QTWIDGETS...

#include <inviwo/core/network/processornetworkevaluationobserver.h>  // for ProcessorNetworkEval...
#include <inviwo/core/network/processornetworkobserver.h>            // for ProcessorNetworkObse...
#include <inviwo/core/properties/propertyownerobserver.h>            // for PropertyOwnerObserver
#include <modules/qtwidgets/inviwodockwidget.h>                      // for InviwoDockWidget

#include <warn/push>
#include <warn/ignore/all>
#include <QEvent>                                                    // for QEvent
#include <QObject>                                                   // for Q_GADGET
#include <QSize>                                                     // for QSize
#include <QWidget>                                                   // for QWidget
#include <qcoreevent.h>                                              // for QEvent::Type, QEvent...

class QPaintEvent;

#include <warn/pop>
#include <cstddef>                                                   // for size_t
#include <string>                                                    // for string
#include <unordered_map>                                             // for unordered_map

class QScrollArea;
class QVBoxLayout;

namespace inviwo {

class InviwoApplication;
class Processor;
class Property;
class PropertyWidgetFactory;

class IVW_MODULE_QTWIDGETS_API PropertyListFrame : public QWidget,
                                                   public ProcessorNetworkObserver,
                                                   public PropertyOwnerObserver {
public:
    PropertyListFrame(QWidget* parent, PropertyWidgetFactory* factory);
    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;
    virtual void paintEvent(QPaintEvent*) override;

    void add(Processor* processor);
    void hide(Processor* processor);
    void remove(Processor* processor);

    void add(Property* property);
    void hide(Property* property);
    void remove(Property* property);

    void clear();

private:
    virtual void onProcessorNetworkWillRemoveProcessor(Processor*) override;
    virtual void onWillRemoveProperty(Property* property, size_t index) override;

    QWidget* get(Processor* processor);
    QWidget* create(Processor* processor);

    QWidget* get(Property* property);
    QWidget* create(Property* property);

    QVBoxLayout* listLayout_;
    std::unordered_map<Processor*, QWidget*> processorMap_;
    std::unordered_map<Property*, QWidget*> propertyMap_;
    PropertyWidgetFactory* factory_;
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

class IVW_MODULE_QTWIDGETS_API PropertyListWidget : public InviwoDockWidget,
                                                    ProcessorNetworkEvaluationObserver {
public:
    PropertyListWidget(QWidget* parent, InviwoApplication* app);
    virtual ~PropertyListWidget();

    void addProcessorProperties(Processor* processor);
    void removeProcessorProperties(Processor* processor);
    void removeAndDeleteProcessorProperties(Processor* processor);

    void addPropertyWidgets(Property* property);
    void removePropertyWidgets(Property* property);
    void removeAndDeletePropertyWidgets(Property* property);

    // Override QWidget
    virtual bool event(QEvent* e) override;

    // ProcessorNetworkEvaluationObserver
    virtual void onProcessorNetworkEvaluationBegin() override;
    virtual void onProcessorNetworkEvaluationEnd() override;

private:
    InviwoApplication* app_;

    PropertyListFrame* frame_;
    QScrollArea* scrollArea_;
};

}  // namespace inviwo
