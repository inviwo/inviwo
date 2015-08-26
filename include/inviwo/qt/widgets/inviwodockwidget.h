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

#ifndef IVW_INVIWODOCKWIDGET_H
#define IVW_INVIWODOCKWIDGET_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <QDockWidget>


class QKeyEvent;
class QLayout;

namespace inviwo {

    class InviwoDockWidgetTitleBar;

class IVW_QTWIDGETS_API InviwoDockWidget : public QDockWidget {
    Q_OBJECT

public:
    InviwoDockWidget(QString title, QWidget* parent);
    virtual ~InviwoDockWidget();
    virtual void showEvent(QShowEvent* showEvent) override;
    virtual void keyPressEvent(QKeyEvent* keyEvent) override;

    void setSticky(bool sticky);
    bool isSticky() const;

    void setContents(QWidget *widget);
    void setContents(QLayout *layout);

protected slots:
    void updateWindowTitle(const QString &string);

private:
    InviwoDockWidgetTitleBar* dockWidgetTitleBar_;
};

} // namespace

#endif // IVW_INVIWODOCKWIDGET_H
