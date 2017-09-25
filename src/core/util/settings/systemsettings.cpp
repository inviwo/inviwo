/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2017 Inviwo Foundation
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
#include <inviwo/core/common/inviwocore.h>

namespace inviwo {

SystemSettings::SystemSettings()
    : Settings("System Settings")
    , applicationUsageMode_("applicationUsageMode", "Application usage mode",
                            {{"applicationMode", "Application Mode", UsageMode::Application},
                             {"developerMode", "Developer Mode", UsageMode::Development}},
                            1)
    , poolSize_("poolSize", "Pool Size", 4, 0, 32)
    , txtEditor_("txtEditor", "Use system text editor", true)
    , enablePortInformation_("enablePortInformation", "Enable port information", true)
    , enablePortInspectors_("enablePortInspectors", "Enable port inspectors", true)
    , portInspectorSize_("portInspectorSize", "Port inspector size", 128, 1, 1024)
#if __APPLE__
    , enableTouchProperty_("enableTouch", "Enable touch", false)
#else
    , enableTouchProperty_("enableTouch", "Enable touch", true)
#endif
    , enablePickingProperty_("enablePicking", "Enable picking", true)
    , enableSoundProperty_("enableSound", "Enable sound", true)
    , useRAMPercentProperty_("useRAMPercent", "Max memory usage (%)", 50, 1, 100)
    , logStackTraceProperty_("logStackTraceProperty", "Error stack trace log", false)
    , btnAllocTestProperty_("allocTest", "Perform Allocation Test")
    , btnSysInfoProperty_("printSysInfo", "Print System Info")
    , followObjectDuringRotation_("followObjectDuringRotation",
                                  "Follow Object During Camera Rotation", true)
    , allocTest_(nullptr) {

    addProperty(applicationUsageMode_);
    addProperty(poolSize_);
    addProperty(txtEditor_);
    addProperty(enablePortInformation_);
    addProperty(enablePortInspectors_);
    addProperty(portInspectorSize_);
    addProperty(enableTouchProperty_);
    addProperty(enablePickingProperty_);
    addProperty(enableSoundProperty_);
    addProperty(useRAMPercentProperty_);
    addProperty(logStackTraceProperty_);
    addProperty(followObjectDuringRotation_);

    logStackTraceProperty_.onChange(this, &SystemSettings::logStacktraceCallback);
    // btnAllocTestProperty_.onChange(this, &SystemSettings::allocationTest);
    // addProperty(&btnAllocTestProperty_);


    auto cores = std::thread::hardware_concurrency();
    if (cores > 0) {
        isDeserializing_ = true;
        poolSize_.setMaxValue(cores);
        poolSize_.set(static_cast<int>(0.5 * cores));
        poolSize_.setCurrentStateAsDefault();
        isDeserializing_ = false;
    }
    load();
}

void SystemSettings::logStacktraceCallback() {
    LogCentral::getPtr()->setLogStacktrace(logStackTraceProperty_.get());
}

void SystemSettings::allocationTest() {
    auto module = InviwoApplication::getPtr()->getModuleByType<InviwoCore>();
    if (!module) return;

    auto sysInfo = getTypeFromVector<SystemCapabilities>(module->getCapabilities());

    if (sysInfo) {
        IntProperty* useRAMPercent =
            dynamic_cast<IntProperty*>(getPropertyByIdentifier("useRAMPercent"));
        glm::u64 memBytesAlloc = sysInfo->getAvailableMemory();  // In Bytes
        LogInfo("Maximum Available Memory is " << util::formatBytesToString(memBytesAlloc));
        memBytesAlloc /= 100;                   // 1% of total available memory
        memBytesAlloc *= useRAMPercent->get();  //?% of total available memory

        try {
            allocTest_ = new glm::u32[static_cast<glm::u32>(memBytesAlloc / 4)];
            LogInfo("Allocated " << util::formatBytesToString(memBytesAlloc) << ", which is "
                                 << useRAMPercent->get() << "% of available memory");
            delete allocTest_;
        } catch (std::bad_alloc&) {
            LogError("Failed allocation of " << util::formatBytesToString(memBytesAlloc)
                                             << ", which is " << useRAMPercent->get()
                                             << "% of available memory");
        }
    }
}

UsageMode SystemSettings::getApplicationUsageMode() const { return applicationUsageMode_.get(); }

}  // namespace inviwo
