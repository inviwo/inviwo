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
    , enablePortInspectorsProperty_("enablePortInspectors", "Enable port inspectors", false)
    , portInspectorSize_("portInspectorSize", "Port inspector size", 128, 1, 1024)
    , enablePickingProperty_("enablePicking", "Enable picking", true)
    , enableSoundProperty_("enableSound", "Enable sound", true)
    , useRAMPercentProperty_("useRAMPercent", "Max memory usage (%)", 50, 1, 100)
    , logStackTraceProperty_("logStackTraceProperty", "Error stack trace log", false)
    , btnAllocTestProperty_("allocTest", "Perform Allocation Test")
    , btnSysInfoProperty_("printSysInfo", "Print System Info")
    , allocTest_(nullptr) {}

SystemSettings::~SystemSettings() {}

void SystemSettings::initialize() {
    applicationUsageModeProperty_.addOption("applicationMode","Application Mode", 0);
    applicationUsageModeProperty_.addOption("developerMode","Developer Mode", 1);
    applicationUsageModeProperty_.setSelectedIndex(1);
    applicationUsageModeProperty_.setCurrentStateAsDefault();
    addProperty(&applicationUsageModeProperty_);
    addProperty(&txtEditorProperty_);
    addProperty(&enablePortInformationProperty_);
    addProperty(&enablePortInspectorsProperty_);
    addProperty(&portInspectorSize_);
    addProperty(&enablePickingProperty_);
    addProperty(&enableSoundProperty_);
    addProperty(&useRAMPercentProperty_);
    addProperty(logStackTraceProperty_);
    logStackTraceProperty_.onChange(this,&SystemSettings::logStacktraceCallback);
    //btnAllocTestProperty_.onChange(this, &SystemSettings::allocationTest);
    //addProperty(&btnAllocTestProperty_);

    InviwoCore* module = InviwoApplication::getPtr()->getModuleByType<InviwoCore>();
    if (module) {
        SystemCapabilities* sysInfo = getTypeFromVector<SystemCapabilities>(module->getCapabilities());

        if (sysInfo) {
            btnSysInfoProperty_.onChange(sysInfo, &SystemCapabilities::printInfo);
            addProperty(&btnSysInfoProperty_);
        }
    }
}

void SystemSettings::deinitialize()  {}

void SystemSettings::logStacktraceCallback() {
    LogCentral::getPtr()->setLogStacktrace(logStackTraceProperty_.get());
}

void SystemSettings::allocationTest() {
    InviwoCore* module = InviwoApplication::getPtr()->getModuleByType<InviwoCore>();
    if (!module) return;

    SystemCapabilities* sysInfo =
        getTypeFromVector<SystemCapabilities>(module->getCapabilities());

    if (sysInfo) {
        IntProperty* useRAMPercent =
            dynamic_cast<IntProperty*>(getPropertyByIdentifier("useRAMPercent"));
        glm::u64 memBytesAlloc = sysInfo->getAvailableMemory();  // In Bytes
        LogInfo("Maximum Available Memory is " << formatBytesToString(memBytesAlloc));
        memBytesAlloc /= 100;  // 1% of total available memory
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

} // namespace
