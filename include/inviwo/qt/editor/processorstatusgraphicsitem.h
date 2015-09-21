/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_PROCESSORSTATUSGRAPHICSITEM_H
#define IVW_PROCESSORSTATUSGRAPHICSITEM_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/editor/editorgrapicsitem.h>
#include <inviwo/core/processors/processorwidgetobserver.h>
#include <inviwo/core/processors/activityindicator.h>
#include <QEvent>
#include <QRectF>

namespace inviwo {

class Processor;

class IVW_QTEDITOR_API ProcessorStatusGraphicsItem : public EditorGraphicsItem,
                                                     public ProcessorWidgetObserver,
                                                     public ActivityIndicatorObserver{
public:
    ProcessorStatusGraphicsItem(QGraphicsRectItem* parent, Processor* processor);
    virtual ~ProcessorStatusGraphicsItem() {}

    void setRunning(bool);

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + ProcessorStatusGraphicsType };
    int type() const override { return Type; }

    // ProcessorWidgetObserver overrides
    virtual void onProcessorWidgetShow(ProcessorWidget*) override;
    virtual void onProcessorWidgetHide(ProcessorWidget*) override;

    virtual void update(const QRectF& rect = QRectF());

protected:
    void paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* widget) override;
    virtual void activityIndicatorChanged(bool active) override;

private:
    enum class State {Invalid, Running, Ready};

    Processor* processor_;
    float size_;
    float lineWidth_;

    State state_;
    State current_;
};

}  // namespace

#endif  // IVW_PROCESSORSTATUSGRAPHICSITEM_H
