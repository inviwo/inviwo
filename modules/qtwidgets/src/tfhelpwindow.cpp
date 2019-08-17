/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/qtwidgets/tfhelpwindow.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QAction>
#include <QMenu>
#include <QVBoxLayout>
#include <QTextEdit>
#include <warn/pop>

namespace inviwo {

TFHelpWindow::TFHelpWindow(QWidget* parent)
    : InviwoDockWidget("Help: Transfer Function Editor", parent, "TFHelpWidget") {
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(60, 60)));  // default size

    auto centralWidget = new QWidget();
    auto vLayout = new QVBoxLayout(centralWidget);
    vLayout->setSpacing(utilqt::refSpacePx(this));
    vLayout->setContentsMargins(0, 0, 0, 0);

    auto textedit = new QTextEdit(this);
    vLayout->addWidget(textedit);
    centralWidget->setLayout(vLayout);
    setWidget(centralWidget);

    textedit->setReadOnly(true);
    textedit->setUndoRedoEnabled(false);
    textedit->setAcceptRichText(true);

    const std::string contents = R"(<h1 style='color:white;'>Transfer Function Editor</h1>
<p>The Transfer Function Editor allows to create and modify a 1D transfer
function (TF) for mapping scalar values to color and opacity. A TF consists of
multiple TF points each associated with a scalar value, i.e. its position, a rgb
color, and an alpha value. The TF is shown in the left area along with the
histogram of the input data. Scalar values are mapped to the horizontal axis
while the vertical axis corresponds to the opacity.</p>

<h3>Adding TF Points</h3>
<p>TF points are added by pressing &lt;CTRL&gt; and the left mouse button or via
the context menu.</p>
<p>Points can also be duplicated via the context menu, which will offset the new
points slightly to the right.</p>

<h3>Deleting TF Points</h3>
<p>To delete a point press &lt;Backspace&gt; or &lt;Delete&gt; or use the
context menu.</p>

<h3>Manipulating TF Points</h3>
<p>Points can be adjusted using the left mouse button or using the Scalar and
Alpha fields on the right-hand side. When using the mouse to move TF points,
&lt;SHIFT&gt; can be used to restrict the movement to either horizontal or
vertical.</p>
<p>Alternatively, the arrow keys move TF points left/right or up/down.
&lt;SHIFT&gt; increases the step size while &lt;ALT&gt; reduces it.</p>

<h3>Selecting Multiple Points</h3>
<p>Multiple points are selected by a rubber band selection or holding
&lt;CTRL&gt; while left clicking on individual points</p>
<p>A selection can be grouped and recalled later via the context menu or using
&lt;CTRL&gt;+&lt;1&gt;,&lt;2&gt;,&lt;3&gt;, ...,&lt;0&gt; for grouping and
&lt;1&gt;,&lt;2&gt;,&lt;3&gt;, ...,&lt;0&gt; without any modifier for
selecting the respective group. Pressing &lt;SHIFT&gt; while selecting a group
will add the elements of the group to the current selection.</p>

<h3>Masking the TF</h3>
<p>Parts of the TF can be masked out, i.e. alpha is implicitly set to zero outside 
the mask. The context menu provides functionality for setting the begin and end points 
of the mask as well as clearing it.</p>

<h3>On Transfer Function Level</h3>
<p>The entire transfer function can be cleared at once, i.e. all TF points are deleted, 
or reset to the properties' default state via the context menu. Furthermore, TFs can be 
exported and imported.</p>)";

    Document doc;
    auto html = doc.append("html");
    html.append("head").append("style",
                               "a { color: #c8ccd0; font-weight: normal;}\n"
                               "body, table, div, p, dl "
                               "{color: #9d9995; background-color: #323235; font: 400 14px/18px"
                               "Calibra, sans-serif;}\n "
                               "h1, h2, h3, h4 {color: #c8ccd0; margin-bottom:3px;}");
    html.append("body", contents, {{"style", "margin: 7px;"}});

    std::string str = doc;
    textedit->setHtml(utilqt::toQString(str));
}

TFMenuHelper::TFMenuHelper() {
    if (utilqt::getApplicationMainWindow()) {
        showAction_ = std::make_unique<QAction>(QString("&Transfer Function"), nullptr);
        QObject::connect(showAction_.get(), &QAction::triggered, this, [this]() {
            auto window = getWindow();
            window->activateWindow();
            window->raise();
            window->show();
        });

        // insert menu entry in the help menu before the "&About"
        auto helpMenu = utilqt::getMenu("&Help", true);
        auto menuActions = helpMenu->actions();
        if (menuActions.empty()) {
            helpMenu->addAction(showAction_.get());
        } else {
            // insert action before last entry
            helpMenu->insertAction(menuActions.last(), showAction_.get());
        }
    }
}

TFMenuHelper::~TFMenuHelper() {
    if (utilqt::getApplicationMainWindow()) {
        // remove menu action from help menu and delete help window since
        // the MainWindow is parent and will not delete the the window until after
        // module has been de-initialized.
        // Destructors will remove the created widgets, actions and signals

        if (auto helpMenu = utilqt::getMenu("&Help")) {
            helpMenu->removeAction(showAction_.get());
        }
    }
}

TFHelpWindow* TFMenuHelper::getWindow() const {
    if (!helpWindow_) {
        auto win = utilqt::getApplicationMainWindow();
        helpWindow_ = std::make_unique<TFHelpWindow>(win);
        helpWindow_->setVisible(false);
        helpWindow_->setFloating(true);
        helpWindow_->loadState();

        // release unique pointer of window when it is about to be destroyed by Qt
        QObject::connect(helpWindow_.get(), &TFHelpWindow::destroyed, this,
                         [this]() { helpWindow_.release(); });
    }

    return helpWindow_.get();
}

QAction* TFMenuHelper::getAction() const { return showAction_.get(); }

void TFMenuHelper::showWindow() const {
    auto window = getWindow();
    window->activateWindow();
    window->raise();
    window->show();
}

}  // namespace inviwo
