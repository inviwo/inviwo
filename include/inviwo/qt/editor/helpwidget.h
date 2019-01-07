/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#ifndef IVW_HELPWIDGET_H
#define IVW_HELPWIDGET_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <modules/qtwidgets/inviwodockwidget.h>

class QObject;
class QHelpEngineCore;
class QResizeEvent;

namespace inviwo {

class QCHFileObserver;
class HelpBrowser;

class IVW_QTEDITOR_API HelpWidget : public InviwoDockWidget {
public:
    HelpWidget(InviwoMainWindow* parent);
    virtual ~HelpWidget();
    HelpWidget(const HelpWidget&) = delete;
    HelpWidget& operator=(const HelpWidget&) = delete;

    void showDocForClassName(std::string className);
    void registerQCHFiles();

protected:
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    void updateDoc();

    InviwoMainWindow* mainwindow_;
    QHelpEngineCore* helpEngine_;
    HelpBrowser* helpBrowser_;
    std::string requested_;
    std::string current_;
    std::unique_ptr<QCHFileObserver> fileObserver_;

    // Called after modules have been registered
    std::shared_ptr<std::function<void()>> onModulesDidRegister_;
    // Called before modules have been unregistered
    std::shared_ptr<std::function<void()>> onModulesWillUnregister_;
};

}  // namespace inviwo

#endif  // IVW_HELPWIDGET_H
