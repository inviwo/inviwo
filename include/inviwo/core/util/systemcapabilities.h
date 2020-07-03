/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2020 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/capabilities.h>
#include <inviwo/core/util/buildinfo.h>
#include <vector>
#include <string>

#ifdef IVW_USE_SIGAR
struct sigar_t;
#endif

namespace inviwo {

class IVW_CORE_API SystemCapabilities : public Capabilities {

public:
    struct OSInfo {
        std::string description;
        int platform;
    };

    struct CPUInfo {
        std::string vendor;
        std::string model;
        size_t mhz;
    };

    struct MemoryInfo {
        size_t total;      // In Bytes
        size_t available;  // In Bytes
    };

    struct DiskInfo {
        std::string diskName;
        std::string diskType;
        size_t total;  // In Bytes
        size_t free;   // In Bytes
    };

    struct ProcessMemoryInfo {
        size_t residentMem;  // In Bytes
        size_t sharedMem;    // In Bytes
        size_t virtualMem;   // In Bytes
    };

    SystemCapabilities();
    virtual ~SystemCapabilities();

    virtual void printInfo() override;

    int numberOfCores() const;

    size_t getAvailableMemory();
    size_t getCurrentResidentMemoryUsage();

    virtual void retrieveStaticInfo() override;
    virtual void retrieveDynamicInfo() override;

    const util::BuildInfo& getBuildInfo() const;

private:
    bool lookupOSInfo();
    bool lookupCPUInfo();
    bool lookupMemoryInfo();
    bool lookupDiskInfo();
    bool lookupProcessMemoryInfo();

    OSInfo infoOS_;
    std::vector<CPUInfo> infoCPUs_;
    MemoryInfo infoRAM_;
    std::vector<DiskInfo> infoDisks_;
    ProcessMemoryInfo infoProcRAM_;
    util::BuildInfo buildInfo_;

    bool successOSInfo_;
    bool successCPUInfo_;
    bool successMemoryInfo_;
    bool successDiskInfo_;
    bool successProcessMemoryInfo_;

#ifdef IVW_USE_SIGAR
    sigar_t* sigar_;
#endif
};

}  // namespace inviwo
