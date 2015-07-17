/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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
#include <inviwo/core/properties/buttonproperty.h>

namespace inviwo {

class IVW_CORE_API SystemSettings : public Settings {

public:
    SystemSettings();
    virtual ~SystemSettings();
    virtual void initialize();
    virtual void deinitialize();

    UsageMode getApplicationUsageMode() const;

    OptionPropertyInt applicationUsageModeProperty_;
    IntProperty poolSize_;
    BoolProperty txtEditorProperty_;
    BoolProperty enablePortInformationProperty_;
    BoolProperty enablePortInspectorsProperty_;
    IntProperty portInspectorSize_;
    BoolProperty enablePickingProperty_;
    BoolProperty enableSoundProperty_;
    IntProperty  useRAMPercentProperty_;
    BoolProperty  logStackTraceProperty_;
    ButtonProperty btnAllocTestProperty_;
    ButtonProperty btnSysInfoProperty_;

    CompositeProperty glslSyntax_;
    IntVec4Property glslTextColor_;
    IntVec4Property glslBackgroundColor_;
    IntVec4Property glslQualifierColor_;
    IntVec4Property glslBuiltinsColor_;
    IntVec4Property glslTypeColor_;
    IntVec4Property glslGlslBuiltinsColor_;
    IntVec4Property glslCommentColor_;
    IntVec4Property glslPreProcessorColor_;

    CompositeProperty pythonSyntax_;
    IntProperty pyFontSize_;
    IntVec4Property pyBGColor_;
    IntVec4Property pyTextColor_;
    IntVec4Property pyTypeColor_;
    IntVec4Property pyCommentsColor_;

protected:
    void logStacktraceCallback();
    void allocationTest();
    glm::u32* allocTest_;
};

} // namespace

#endif // IVW_SYSTEMSETTINGS_H
