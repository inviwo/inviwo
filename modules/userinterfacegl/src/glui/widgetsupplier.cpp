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

#include <modules/userinterfacegl/glui/widgetsupplier.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <modules/userinterfacegl/userinterfaceglmodule.h>

namespace inviwo {

namespace glui {

WidgetSupplier::WidgetSupplier(UserInterfaceGLModule& uiGLModule) : uiGLModule_(uiGLModule) {}

WidgetSupplier::WidgetSupplier(InviwoApplication* app)
    : uiGLModule_{[app]() -> UserInterfaceGLModule& {
        if (app) {
            if (auto uiGLModule = app->getModuleByType<UserInterfaceGLModule>()) {
                return *uiGLModule;
            }
        }
        throw Exception("Was not able to find the User Interface GL module",
                        IVW_CONTEXT_CUSTOM("UserInterfaceGLModule"));
    }()} {}

WidgetSupplier::~WidgetSupplier() { unregisterAll(); }

WidgetFactory& WidgetSupplier::getGLUIWidgetFactory() { return uiGLModule_.getGLUIWidgetFactory(); }

void WidgetSupplier::unregisterAll() {
    for (auto& elem : widgets_) {
        uiGLModule_.getGLUIWidgetFactory().unRegisterObject(elem.get());
    }
    widgets_.clear();
}

}  // namespace glui

}  // namespace inviwo
