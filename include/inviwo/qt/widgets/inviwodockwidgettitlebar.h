/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#ifndef IVW_INVIWODOCKWIDGETTITLEBAR_H
#define IVW_INVIWODOCKWIDGETTITLEBAR_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <warn/pop>
class QPaintEvent;
class QDockWidget;
class QToolButton;
class QLabel;

namespace inviwo {

/*! \class InviwoDockWidgetTitleBar
\brief Custom title bar widget for QDockWidget derived from QWidget.

The title bar widget contains a label showing the window title of the 
parent QDockWidget and buttons for the sticky mode, dock/undock, and close.
The sticky mode determines whether the floating dock widget can be docked
to prevent involuntary docking when moving the dock widget. Docking via the 
dock button or double clicking the title bar is still possible.

Note: adding a custom title bar to a QDockWidget removes the window decoration!
*/
class IVW_QTWIDGETS_API InviwoDockWidgetTitleBar : public QWidget {
    #include <warn/push>
    #include <warn/ignore/all>
    Q_OBJECT
    #include <warn/pop>
public:
    InviwoDockWidgetTitleBar(QWidget *parent=nullptr);
    virtual ~InviwoDockWidgetTitleBar();

    virtual void paintEvent(QPaintEvent *) override;
    

    void setLabel(const QString &str);

    void setSticky(bool toggle);
    bool isSticky() const;

public slots:
    void floating(bool floating);
protected slots:
    void stickyBtnToggled(bool toggle);
    void floatBtnClicked();

private:
    QDockWidget *parent_;

    QLabel* label_;
    QToolButton *stickyBtn_;
    QToolButton *floatBtn_;
    Qt::DockWidgetAreas allowedDockAreas_;
};

} // namespace

#endif // IVW_INVIWODOCKWIDGETTITLEBAR_H