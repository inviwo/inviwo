/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#pragma once

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <QColor>   // for QColor
#include <QImage>   // for QImage
#include <QObject>  // for slots, Q_OBJECT, signals
#include <QPoint>   // for QPoint
#include <QRegion>  // for QRegion
#include <QSize>    // for QSize
#include <QWidget>  // for QWidget

class QPainter;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;

namespace inviwo {

class IVW_MODULE_QTWIDGETS_API ColorWheel : public QWidget {
    Q_OBJECT
public:
    explicit ColorWheel(const QSize& initialSize = QSize(200, 200), QWidget* parent = 0);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    QColor color() const;

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
    void hueChanged(int hue);
    void svChanged(const QColor& newcolor);

private:
    void setColorFromWheel(const QPoint& point);
    void setColorFromSquare(const QPoint& point);

    void drawOuterRing(QPainter& painter);
    void drawIndicator(QPainter& painter);
    void drawPicker(QPainter& painter);

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

}  // namespace inviwo
