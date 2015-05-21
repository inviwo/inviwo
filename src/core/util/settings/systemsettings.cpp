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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/systemcapabilities.h>
#include <inviwo/core/util/formatconversion.h>

namespace inviwo {

SystemSettings::SystemSettings()
    : Settings("System Settings")
    , applicationUsageModeProperty_("applicationUsageMode", "Application usage mode")
    , txtEditorProperty_("txtEditor", "Use system text editor", true)
    , enablePortInformationProperty_("enablePortInformation", "Enable port information", true)
    , enablePortInspectorsProperty_("enablePortInspectors", "Enable port inspectors", true)
    , portInspectorSize_("portInspectorSize", "Port inspector size", 128, 1, 1024)
    , enablePickingProperty_("enablePicking", "Enable picking", true)
    , enableSoundProperty_("enableSound", "Enable sound", true)
    , useRAMPercentProperty_("useRAMPercent", "Max memory usage (%)", 50, 1, 100)
    , logStackTraceProperty_("logStackTraceProperty", "Error stack trace log", false)
    , btnAllocTestProperty_("allocTest", "Perform Allocation Test")
    , btnSysInfoProperty_("printSysInfo", "Print System Info")

    , glslSyntax_("glslSyntax", "GLSL Syntax Highlighting")
    , glslTextColor_("glslTextColor", "Text", ivec4(0xAA, 0xAA, 0xAA, 255), ivec4(0, 0, 0, 1),
                     ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1), INVALID_OUTPUT,
                     PropertySemantics::Color)
    , glslBackgroundColor_("glslBackgroundColor", "Background", ivec4(0x4D, 0x4D, 0x4D, 255),
                           ivec4(0, 0, 0, 1), ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1),
                           INVALID_OUTPUT, PropertySemantics::Color)
    , glslQualifierColor_("glslQualifierColor", "Qualifiers", ivec4(0x7D, 0xB4, 0xDF, 255),
                          ivec4(0, 0, 0, 1), ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1),
                          INVALID_OUTPUT, PropertySemantics::Color)
    , glslBuiltinsColor_("glslBultinsColor", "Builtins", ivec4(0x1F, 0xF0, 0x7F, 255),
                         ivec4(0, 0, 0, 1), ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1),
                         INVALID_OUTPUT, PropertySemantics::Color)
    , glslTypeColor_("glslTypeColor", "Types", ivec4(0x56, 0x9C, 0xD6, 255), ivec4(0, 0, 0, 1),
                     ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1), INVALID_OUTPUT,
                     PropertySemantics::Color)
    , glslGlslBuiltinsColor_("glslGlslBultinsColor", "GLSL Builtins", ivec4(0xFF, 0x80, 0x00, 255),
                             ivec4(0, 0, 0, 1), ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1),
                             INVALID_OUTPUT, PropertySemantics::Color)
    , glslCommentColor_("glslCommentColor", "Comments", ivec4(0x60, 0x8B, 0x4E, 255),
                        ivec4(0, 0, 0, 1), ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1),
                        INVALID_OUTPUT, PropertySemantics::Color)
    , glslPreProcessorColor_("glslPreProcessorColor", "Pre Processor", ivec4(0x9B, 0x9B, 0x9B, 255),
                             ivec4(0, 0, 0, 1), ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1),
                             INVALID_OUTPUT, PropertySemantics::Color)

    , pythonSyntax_("pythonSyntax_", "Python Syntax Highlighting")
    , pyFontSize_("pyFontSize_" , "Font Size" , 11 , 1, 72)
    , pyBGColor_("pyBGColor", "Background", ivec4(0xb0, 0xb0, 0xbc, 255), ivec4(0, 0, 0, 1),
                 ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1), INVALID_OUTPUT, 
                 PropertySemantics::Color)
    , pyTextColor_("pyTextColor", "Text", ivec4(0x11, 0x11, 0x11, 255), ivec4(0, 0, 0, 1),
                   ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1), INVALID_OUTPUT,
                   PropertySemantics::Color)
    , pyTypeColor_("pyTypeColor", "Types", ivec4(0x14, 0x3C, 0xA6, 255), ivec4(0, 0, 0, 1),
                   ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1), INVALID_OUTPUT,
                   PropertySemantics::Color)
    , pyCommentsColor_("pyCommentsColor", "Comments", ivec4(0x00, 0x66, 0x00, 255),
                       ivec4(0, 0, 0, 1), ivec4(255, 255, 255, 1), ivec4(1, 1, 1, 1),
                       INVALID_OUTPUT, PropertySemantics::Color)

    , allocTest_(nullptr) {
    applicationUsageModeProperty_.addOption("applicationMode", "Application Mode", 0);
    applicationUsageModeProperty_.addOption("developerMode", "Developer Mode", 1);
    applicationUsageModeProperty_.setSelectedIndex(1);
    applicationUsageModeProperty_.setCurrentStateAsDefault();
    addProperty(applicationUsageModeProperty_);
    addProperty(txtEditorProperty_);
    addProperty(enablePortInformationProperty_);
    addProperty(enablePortInspectorsProperty_);
    addProperty(portInspectorSize_);
    addProperty(enablePickingProperty_);
    addProperty(enableSoundProperty_);
    addProperty(useRAMPercentProperty_);
    addProperty(logStackTraceProperty_);
    addProperty(pythonSyntax_);
    addProperty(glslSyntax_);

    glslSyntax_.addProperty(glslBackgroundColor_);
    glslSyntax_.addProperty(glslTextColor_);
    glslSyntax_.addProperty(glslCommentColor_);
    glslSyntax_.addProperty(glslTypeColor_);
    glslSyntax_.addProperty(glslQualifierColor_);
    glslSyntax_.addProperty(glslBuiltinsColor_);
    glslSyntax_.addProperty(glslGlslBuiltinsColor_);
    glslSyntax_.addProperty(glslPreProcessorColor_);

    pythonSyntax_.addProperty(pyFontSize_); 
    pythonSyntax_.addProperty(pyBGColor_); 
    pythonSyntax_.addProperty(pyTextColor_);
    pythonSyntax_.addProperty(pyCommentsColor_);
    pythonSyntax_.addProperty(pyTypeColor_);

    logStackTraceProperty_.onChange(this, &SystemSettings::logStacktraceCallback);
    // btnAllocTestProperty_.onChange(this, &SystemSettings::allocationTest);
    // addProperty(&btnAllocTestProperty_);
}

SystemSettings::~SystemSettings() {}

void SystemSettings::initialize() {
    pythonSyntax_.setVisible(false);
    glslSyntax_.setVisible(false);

    InviwoCore* module = InviwoApplication::getPtr()->getModuleByType<InviwoCore>();
    if (module) {
        SystemCapabilities* sysInfo =
            getTypeFromVector<SystemCapabilities>(module->getCapabilities());
        if (sysInfo) {
            btnSysInfoProperty_.onChange(sysInfo, &SystemCapabilities::printInfo);
            addProperty(btnSysInfoProperty_);
        }
    }
}

void SystemSettings::deinitialize() {}

void SystemSettings::logStacktraceCallback() {
    LogCentral::getPtr()->setLogStacktrace(logStackTraceProperty_.get());
}

void SystemSettings::allocationTest() {
    InviwoCore* module = InviwoApplication::getPtr()->getModuleByType<InviwoCore>();
    if (!module) return;

    SystemCapabilities* sysInfo = getTypeFromVector<SystemCapabilities>(module->getCapabilities());

    if (sysInfo) {
        IntProperty* useRAMPercent =
            dynamic_cast<IntProperty*>(getPropertyByIdentifier("useRAMPercent"));
        glm::u64 memBytesAlloc = sysInfo->getAvailableMemory();  // In Bytes
        LogInfo("Maximum Available Memory is " << formatBytesToString(memBytesAlloc));
        memBytesAlloc /= 100;                   // 1% of total available memory
        memBytesAlloc *= useRAMPercent->get();  //?% of total available memory

        try {
            allocTest_ = new glm::u32[static_cast<glm::u32>(memBytesAlloc / 4)];
            LogInfo("Allocated " << formatBytesToString(memBytesAlloc) << ", which is "
                                 << useRAMPercent->get() << "% of available memory");
            delete allocTest_;
        } catch (std::bad_alloc&) {
            LogError("Failed allocation of " << formatBytesToString(memBytesAlloc) << ", which is "
                                             << useRAMPercent->get() << "% of available memory");
        }
    }
}

UsageMode SystemSettings::getApplicationUsageMode() const {
    return static_cast<UsageMode>(applicationUsageModeProperty_.get());
}

}  // namespace
