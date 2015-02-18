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

#ifndef IVW_ANGLERADIUSWIDGET_H
#define IVW_ANGLERADIUSWIDGET_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <QWidget>

namespace inviwo {
/** \class AngleRadiusWidget
 * The AngleRadiusWidget provides an interface to edit an angle in [0 2pi) 
 * as well as a radius within given bounds [minRadius maxRadius].
 * 
 * @see AnglePropertyWidgetQt
 */
class IVW_QTWIDGETS_API AngleRadiusWidget : public QWidget {
    Q_OBJECT
public:
    AngleRadiusWidget(QWidget*);
    virtual ~AngleRadiusWidget() {};

    virtual void paintEvent(QPaintEvent *);

    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

    void setAngle(double angle);
    double getAngle() const { return angle_;}
    void setRadius(double radius);
    double getRadius() const { return radius_;}

    void setMinMaxAngle(double minAngle, double maxAngle);
    void setMinMaxRadius(double minRadius, double maxRadius);

    void setMinAngle(double angle);
    double getMinAngle() const { return minAngle_;}
    void setMaxAngle(double angle);
    double getMaxAngle() const { return maxAngle_;}

    void setMinRadius(double radius);
    double getMinRadius() const { return minRadius_;}
    void setMaxRadius(double radius);
    double getMaxRadius() const { return maxRadius_;}

    QSize sizeHint() const;
    QSize minimumSizeHint() const;
signals:
    void angleChanged();
    void radiusChanged();
    void angleMinMaxChanged();
    void radiusMinMaxChanged();
private:
    /** 
     * \brief Outer bounds of circle.
     *
     * @return int Radius in pixes
     */
    int getMaxPixelSpaceRadius() const;
    /** 
     * Calculates and sets the angle and radius from the center
     * of the widget to the position.
     * 
     * @param const QPoint & pos given in pixels
     */
    void setAngleRadiusAtPosition(const QPoint& pos);

    double angle_;
    double radius_;
    double minAngle_;
    double maxAngle_;
    double minRadius_;
    double maxRadius_;




};

}  // namespace

#endif  // IVW_ANGLERADIUSWIDGET_H