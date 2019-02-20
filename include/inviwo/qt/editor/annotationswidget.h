/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>

#include <inviwo/qt/editor/workspaceannotationsqt.h>
#include <modules/qtwidgets/inviwodockwidget.h>

#include <memory>
#include <functional>

class QVBoxLayout;
class QScrollArea;
class QString;
class QVBoxLayout;

namespace inviwo {

class InviwoMainWindow;

class IVW_QTEDITOR_API AnnotationsWidget : public InviwoDockWidget {
public:
    AnnotationsWidget(const QString& title, InviwoMainWindow* parent);
    AnnotationsWidget(InviwoMainWindow* parent);
    virtual ~AnnotationsWidget();

    WorkspaceAnnotationsQt& getAnnotations();
    const WorkspaceAnnotationsQt& getAnnotations() const;

protected:
    void updateWidget();

    InviwoMainWindow* mainwindow_;
    WorkspaceAnnotationsQt annotations_;
    QVBoxLayout* layout_ = nullptr;
    QWidget* mainWidget_ = nullptr;
    QScrollArea* scrollArea_ = nullptr;
    ///< Called after modules have been registered
    std::shared_ptr<std::function<void()>> onModulesDidRegister_;
    ///< Called before modules have been unregistered
    std::shared_ptr<std::function<void()>> onModulesWillUnregister_;
};

}  // namespace inviwo
