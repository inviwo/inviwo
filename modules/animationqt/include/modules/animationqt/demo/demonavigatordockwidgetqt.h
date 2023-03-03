/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#include <modules/animationqt/animationqtmoduledefine.h>
#include <modules/animation/demo/democontrollerobserver.h>
#include <modules/animation/demo/democontroller.h>
#include <inviwo/core/common/inviwoapplicationutil.h>
#include <inviwo/core/properties/fileproperty.h>
#include <modules/qtwidgets/inviwodockwidget.h>

class QToolButton;

namespace inviwo {

namespace animation {

class IVW_MODULE_ANIMATIONQT_API DemoNavigatorDockWidgetQt : public InviwoDockWidget,
                                                             public DemoControllerObserver,
                                                             public PropertyOwner {
public:
    DemoNavigatorDockWidgetQt(DemoController& controller, const std::string& widgetName,
                              QWidget* parent,
                              InviwoApplication* app = util::getInviwoApplication());
    virtual ~DemoNavigatorDockWidgetQt();

    virtual InviwoApplication* getInviwoApplication() override;

protected:
    DemoController& controller_;
    InviwoApplication* app_;

    QAction* btnLast_;
    QAction* btnNext_;
    QAction* btnBegin_;
    QAction* btnEnd_;
};

}  // namespace animation

}  // namespace inviwo
