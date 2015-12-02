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

/*********************************************************************************
* Parts of code is courtesy of (viewed latest at 2014-02-18):
* https://github.com/liuyanghejerry/Qt-Plus
* README text:
* Welcome to the small world of QtPlus.info.
* All the code here is free for you.
 *********************************************************************************/

#ifndef IVW_COLORWHEEL_H
#define IVW_COLORWHEEL_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QColor>
#include <QPainter>
#include <warn/pop>

class QMouseEvent;
class QResizeEvent;
class QPaintEvent;

namespace inviwo {

class IVW_QTWIDGETS_API ColorWheel : public QWidget {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    explicit ColorWheel(QWidget* parent = 0);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    QColor color();

signals:
    void colorChange(const QColor& color);

public slots:
    void setColor(const QColor& color);

protected:
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void resizeEvent(QResizeEvent* event);
    void paintEvent(QPaintEvent*);

protected slots:
    void hueChanged(const int& hue);
    void svChanged(const QColor& newcolor);

private:
    QColor posColor(const QPoint& point);
    void drawOuterRing(QPainter &painter);
    void drawIndicator(QPainter &painter);
    void drawPicker(QPainter &painter);
    
    void updateSquareImage(const int& hue);

    const QSize initSize_;
    QImage squareImage_;
    bool mouseDown_;
    QPoint lastPos_;
    int margin_;
    int wheelWidth_;
    QRegion wheelRegion_;
    QRegion squareRegion_;
    QColor currentColor_;
    bool inWheel_;
    bool inSquare_;
};

} //namespace

#endif // IVW_COLORWHEEL_H
