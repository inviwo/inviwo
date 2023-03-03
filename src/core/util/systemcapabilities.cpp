/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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

#include <inviwo/core/util/systemcapabilities.h>
#include <inviwo/core/util/formatconversion.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/filesystem.h>

#include <sstream>
#include <iomanip>

namespace inviwo {

SystemCapabilities::SystemCapabilities() {
    retrieveStaticInfo();
    retrieveDynamicInfo();
}

SystemCapabilities::~SystemCapabilities() {
}

void SystemCapabilities::retrieveStaticInfo() {
    successOSInfo_ = lookupOSInfo();
    buildInfo_ = util::getBuildInfo();
}

void SystemCapabilities::retrieveDynamicInfo() {
    successCPUInfo_ = lookupCPUInfo();
    successMemoryInfo_ = lookupMemoryInfo();
    successDiskInfo_ = lookupDiskInfo();
    successProcessMemoryInfo_ = lookupProcessMemoryInfo();
}

const util::BuildInfo& SystemCapabilities::getBuildInfo() const { return buildInfo_; }

bool SystemCapabilities::lookupOSInfo() {
    return false;
}

bool SystemCapabilities::lookupCPUInfo() {
    infoCPUs_.clear();
    return false;
}

bool SystemCapabilities::lookupMemoryInfo() {
    return false;
}

bool SystemCapabilities::lookupDiskInfo() {
    infoDisks_.clear();
    return false;
}

bool SystemCapabilities::lookupProcessMemoryInfo() {
    return false;
}

void SystemCapabilities::printInfo() {
    retrieveDynamicInfo();

    // Try to retrieve operating system information
    if (successOSInfo_) {
        LogInfoCustom("SystemInfo",
                      "OS: " << infoOS_.description << " " << infoOS_.platform << "-bit");
    } else {
        LogInfoCustom("SystemInfo", "OS: Info could not be retrieved");
    }

    // Try to retrieve CPU information
    if (successCPUInfo_) {
        std::string prevCPU = std::string("");
        std::ostringstream cpuStream;
        unsigned long count = 0;
        for (unsigned long i = 0; i < infoCPUs_.size(); i++) {
            cpuStream.str("");
            cpuStream << infoCPUs_[i].vendor << " " << infoCPUs_[i].model;
            if (prevCPU != cpuStream.str()) {
                if (!prevCPU.empty()) {
                    if (count > 0) {
                        LogInfoCustom("SystemInfo",
                                      "CPU " << i + 1 << "-" << i + 1 + count << ": " << prevCPU);
                    } else {
                        LogInfoCustom("SystemInfo", "CPU " << i + 1 << ": " << prevCPU);
                    }
                }
                prevCPU = cpuStream.str();
                count = 0;
            } else {
                count++;
            }
        }
        if (!prevCPU.empty()) {
            if (count > 0) {
                LogInfoCustom("SystemInfo", "CPU " << infoCPUs_.size() - count << "-"
                                                   << infoCPUs_.size() << ": " << prevCPU);
            } else {
                LogInfoCustom("SystemInfo", "CPU " << infoCPUs_.size() << ": " << prevCPU);
            }
        }
    } else {
        LogInfoCustom("SystemInfo", "CPU: Info could not be retrieved");
    }

    // Try to retrieve memory information
    if (successMemoryInfo_) {
        LogInfoCustom("SystemInfo", "RAM: Total - "
                                        << util::formatBytesToString(infoRAM_.total)
                                        << ", Available - "
                                        << util::formatBytesToString(infoRAM_.available));
    } else {
        LogInfoCustom("SystemInfo", "RAM: Info could not be retrieved");
    }

    // Try to retrieve Disk information
    if (successDiskInfo_) {
        for (auto& elem : infoDisks_)
            LogInfoCustom("SystemInfo", "Disk: " << elem.diskName << " Total - "
                                                 << util::formatBytesToString(elem.total)
                                                 << ", Free - "
                                                 << util::formatBytesToString(elem.free));
    } else {
        LogInfoCustom("SystemInfo", "Disk: Info could not be retrieved");
    }

    // Try to retrieve this process memory information
    if (successProcessMemoryInfo_) {
        LogInfoCustom("SystemInfo",
                      "Processor Memory: Resident - "
                          << util::formatBytesToString(infoProcRAM_.residentMem) << ", Shared - "
                          << util::formatBytesToString(infoProcRAM_.sharedMem) << ", Virtual - "
                          << util::formatBytesToString(infoProcRAM_.virtualMem));
    } else {
        LogInfoCustom("SystemInfo", "Processor Memory: Info could not be retrieved");
    }
}

int SystemCapabilities::numberOfCores() const {
    if (successCPUInfo_) {
        return static_cast<int>(infoCPUs_.size());
    } else {
        return -1;
    }
}

size_t SystemCapabilities::getAvailableMemory() {
    successMemoryInfo_ = lookupMemoryInfo();

    if (successMemoryInfo_) {
        return infoRAM_.available;
    } else {
        return 0;
    }
}

size_t SystemCapabilities::getCurrentResidentMemoryUsage() {
    successProcessMemoryInfo_ = lookupProcessMemoryInfo();

    if (successMemoryInfo_) {
        return infoProcRAM_.residentMem;
    } else {
        return 0;
    }
}

}  // namespace inviwo
