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

#ifndef IVW_LIGHTPOSITIONWIDGETQT_H
#define IVW_LIGHTPOSITIONWIDGETQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QLabel>
#include <QRadialGradient>
#include <QPixmap>
#include <QPainter>
#include <QBrush>
#include <QMouseEvent>
#include <QPoint>
#include <QtCore/qmath.h>
#include <warn/pop>

#include <inviwo/core/common/inviwo.h>

namespace inviwo {

class IVW_QTWIDGETS_API LightPositionWidgetQt : public QLabel {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>


public:
    LightPositionWidgetQt();
    void setPosition(const vec3& positionVector);
    vec3 getPosition() const;
    virtual ~LightPositionWidgetQt();

    float getX() const;;
    float getY() const;;
    float getZ() const;;

    void setRadius(float radius);
    float getRadius() const;

signals:
    void positionChanged();

protected:
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);

private:
    void generateWidget();
    void setNewPosition(QMouseEvent* event);

    QPixmap* gradientPixmap_;
    QRadialGradient* gradient_;
    
    QPainter* painter_;
    bool mouseDown_;

    float radius_;
    float theta_;
    float phi_;

};

}//namespace

#endif // IVW_INTSLIDERQT_H