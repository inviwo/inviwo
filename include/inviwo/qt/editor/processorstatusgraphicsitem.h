/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/editor/editorgrapicsitem.h>
#include <inviwo/core/processors/activityindicator.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QEvent>
#include <QRectF>
#include <warn/pop>

namespace inviwo {

class Processor;

class IVW_QTEDITOR_API ProcessorStatusGraphicsItem : public EditorGraphicsItem,
                                                     public ActivityIndicatorObserver {
public:
    ProcessorStatusGraphicsItem(QGraphicsRectItem* parent, Processor* processor);
    virtual ~ProcessorStatusGraphicsItem() = default;

    void updateState(bool running = false);

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = static_cast<int>(UserType) + static_cast<int>(ProcessorStatusGraphicsType) };
    int type() const override { return Type; }

    void update(const QRectF& rect = QRectF());

protected:
    void paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* widget) override;
    virtual void activityIndicatorChanged(bool active) override;

private:
    enum class State { Error, Invalid, Running, Ready };
    static constexpr float size_{10.0f};
    static constexpr float lineWidth_{1.0f};

    Processor* processor_;

    State state_;
    State current_;
};

}  // namespace inviwo
