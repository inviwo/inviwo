/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#ifndef IVW_SYSTEMSETTINGS_H
#define IVW_SYSTEMSETTINGS_H

#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>

namespace inviwo {

class InviwoApplication;

/**
 * System settings, owned by the application, loaded before all the factories so we can't use any
 * dynamic properties here
 */
class IVW_CORE_API SystemSettings : public Settings {
public:
    SystemSettings(InviwoApplication* app);
    StringProperty workspaceAuthor_;
    TemplateOptionProperty<UsageMode> applicationUsageMode_;
    IntSizeTProperty poolSize_;
    BoolProperty enablePortInspectors_;
    IntProperty portInspectorSize_;
    BoolProperty enableTouchProperty_;
    BoolProperty enablePickingProperty_;
    BoolProperty enableSoundProperty_;
    BoolProperty logStackTraceProperty_;
    BoolProperty followObjectDuringRotation_;
    BoolProperty runtimeModuleReloading_;
    BoolProperty enableResourceManager_;
    TemplateOptionProperty<MessageBreakLevel> breakOnMessage_;
    BoolProperty breakOnException_;
    BoolProperty stackTraceInException_;

    static size_t defaultPoolSize();
};

}  // namespace inviwo

#endif  // IVW_SYSTEMSETTINGS_H
