/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QDockWidget>
#include <warn/pop>

class QKeyEvent;
class QLayout;

namespace inviwo {

class InviwoDockWidgetTitleBar;

/**
 * \brief dock widget with a custom title bar and sticky status. Docking while dragging the widget
 * is prevented if the sticky flag is false. In case the window title of the dock widget contains
 * the string "[*]", this string will be replaced with either "*" or nothing based on the
 * isWindowModified() state of the widget.
 *
 * \see QWidget::setWindowTitle
 */
class IVW_MODULE_QTWIDGETS_API InviwoDockWidget : public QDockWidget {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>

public:
    InviwoDockWidget(QString title, QWidget* parent);
    InviwoDockWidget(QString title, QWidget* parent, QString objname);
    virtual ~InviwoDockWidget();
    virtual void showEvent(QShowEvent* showEvent) override;
    virtual void keyPressEvent(QKeyEvent* keyEvent) override;

    void setSticky(bool sticky);
    bool isSticky() const;

    void setContents(QWidget* widget);
    void setContents(QLayout* layout);

    /**
     * Save state related to the dock widget, called in the close event.
     * Uses the objectName as settings group
     */
    virtual void saveState();

    /**
     * Load state related to the dock widget, called in the constructor. If you overload this
     * function you must also call it in your constructor.
     * Uses the objectName as settings group
     */
    virtual void loadState();

signals:
    void stickyFlagChanged(bool sticky);

protected:
    virtual void closeEvent(QCloseEvent* event) override;

private:
    InviwoDockWidgetTitleBar* dockWidgetTitleBar_;
};

}  // namespace inviwo

#endif  // IVW_INVIWODOCKWIDGET_H
