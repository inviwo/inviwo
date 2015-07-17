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

#ifndef IVW_SYSTEMCAPABILITIES_H
#define IVW_SYSTEMCAPABILITIES_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/capabilities.h>
#include <vector>
#include <string>

#ifdef IVW_SIGAR
struct sigar_t;
#endif

namespace inviwo {

class IVW_CORE_API SystemCapabilities : public Capabilities  {

public:
    struct OSInfo {
        std::string description;
        int platform;
    };

    struct CPUInfo {
        std::string vendor;
        std::string model;
        glm::u64 mhz;
    };

    struct MemoryInfo {
        glm::u64 total; //In Bytes
        glm::u64 available; //In Bytes
    };

    struct DiskInfo {
        std::string diskName;
        std::string diskType;
        glm::u64 total; //In Bytes
        glm::u64 free; //In Bytes
    };

    struct ProcessMemoryInfo {
        glm::u64 residentMem; //In Bytes
        glm::u64 sharedMem; //In Bytes
        glm::u64 virtualMem; //In Bytes
    };

    SystemCapabilities();
    virtual ~SystemCapabilities();

    void printInfo();

    int numberOfCores() const;

    bool canAllocate(glm::u64 dataSize, glm::u8 percentageOfAvailableMemory = 100);
    uvec3 calculateOptimalBrickSize(uvec3 dimensions, size_t formatSizeInBytes, glm::u8 percentageOfAvailableMemory = 100);

    glm::u64 getAvailableMemory();
    glm::u64 getCurrentResidentMemoryUsage();
protected:
    void retrieveStaticInfo();
    void retrieveDynamicInfo();

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

    bool successOSInfo_;
    bool successCPUInfo_;
    bool successMemoryInfo_;
    bool successDiskInfo_;
    bool successProcessMemoryInfo_;

#ifdef IVW_SIGAR
    sigar_t *sigar_;
#endif
};

} // namespace

#endif // IVW_SYSTEMCAPABILITIES_H
