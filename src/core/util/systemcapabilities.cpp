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

#include <inviwo/core/util/systemcapabilities.h>
#include <inviwo/core/util/formatconversion.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/assertion.h>
#ifdef IVW_SIGAR
#include <sigar/include/sigar.h>
#endif

#ifdef IVW_EMBED_BUILDINFO
#  include <inviwo_buildinfo.h>
#else
#  include <fstream>
#  include <inviwo/core/common/inviwoapplication.h>
#endif // IVW_EMBED_BUILDINFO

#include <sstream>
#include <iomanip>

namespace inviwo {

SystemCapabilities::SystemCapabilities() {
#ifdef IVW_SIGAR
    sigar_open(&sigar_);
#endif
}

SystemCapabilities::~SystemCapabilities() {
#ifdef IVW_SIGAR
    sigar_close(sigar_);
#endif
}

bool SystemCapabilities::canAllocate(glm::u64 dataSize, glm::u8 percentageOfAvailableMemory) {
    return getAvailableMemory() * percentageOfAvailableMemory / 100 >= dataSize;
}

uvec3 SystemCapabilities::calculateOptimalBrickSize(uvec3 dimensions, size_t formatSizeInBytes,
                                                    glm::u8 percentageOfAvailableMemory) {
    uvec3 dim = dimensions;

    while (
        !canAllocate(getMemorySizeInBytes(dim, formatSizeInBytes), percentageOfAvailableMemory)) {
        int theMaxDim = (dim.x > dim.y ? (dim.x > dim.z ? 0 : 2) : (dim.y > dim.z ? 1 : 2));

        if (dim[theMaxDim] % 2 != 0) dim[theMaxDim]++;  // Make the dim we are dividing even

        dim[theMaxDim] /= 2;
    }

    return dim;
}

void SystemCapabilities::retrieveStaticInfo() {
    successOSInfo_ = lookupOSInfo();
#ifdef IVW_EMBED_BUILDINFO
    // retrieve build info from header file
    buildInfo_.year = buildinfo::year;
    buildInfo_.month = buildinfo::month;
    buildInfo_.day = buildinfo::day;
    buildInfo_.hour = buildinfo::hour;
    buildInfo_.minute = buildinfo::minute;
    buildInfo_.second = buildinfo::second;
    buildInfo_.year = buildinfo::year;
    buildInfo_.githashes.assign(buildinfo::githashes.begin(), buildinfo::githashes.end());
#else
    // retrieve build info from ini file
    readBuildInfoFromIni();

#endif // IVW_EMBED_BUILDINFO
}

void SystemCapabilities::retrieveDynamicInfo() {
    successCPUInfo_ = lookupCPUInfo();
    successMemoryInfo_ = lookupMemoryInfo();
    successDiskInfo_ = lookupDiskInfo();
    successProcessMemoryInfo_ = lookupProcessMemoryInfo();
}



std::string SystemCapabilities::getBuildDateString() const {
    std::stringstream ss;
    ss << getBuildTimeYear() << "-" << std::setfill('0') << std::setw(2) << getBuildTimeMonth()
        << "-" << std::setfill('0') << std::setw(2) << getBuildTimeDay() << " "
        << std::setfill('0') << std::setw(2) << getBuildTimeHour() << ":" << std::setfill('0')
        << std::setw(2) << getBuildTimeMinute() << ":" << std::setfill('0') << std::setw(2)
        << getBuildTimeSecond();
    return ss.str();
}

const int SystemCapabilities::getBuildTimeYear() const {
    return buildInfo_.year;
}

const int SystemCapabilities::getBuildTimeMonth() const {
    return buildInfo_.month;
}

const int SystemCapabilities::getBuildTimeDay() const {
    return buildInfo_.day;
}

const int SystemCapabilities::getBuildTimeHour() const {
    return buildInfo_.hour;
}

const int SystemCapabilities::getBuildTimeMinute() const {
    return buildInfo_.minute;
}

const int SystemCapabilities::getBuildTimeSecond() const {
    return buildInfo_.second;
}

const std::size_t SystemCapabilities::getGitNumberOfHashes() const {
    return buildInfo_.githashes.size();
}

const std::pair<std::string, std::string>& SystemCapabilities::getGitHash(std::size_t i) const {
    ivwAssert(i < getGitNumberOfHashes(), "index out of bounds");
    return buildInfo_.githashes[i];
}

bool SystemCapabilities::lookupOSInfo() {
#ifdef IVW_SIGAR
    sigar_sys_info_t systeminfo;
    int status = sigar_sys_info_get(sigar_, &systeminfo);

    if (status == SIGAR_OK) {
        infoOS_.description = std::string(systeminfo.description);

        if (strcmp(systeminfo.arch, "x86") == 0)
            infoOS_.platform = 32;
        else
            infoOS_.platform = 64;

        return true;
    } else {
        return false;
    }

#else
    return false;
#endif
    
}

bool SystemCapabilities::lookupCPUInfo() {
    infoCPUs_.clear();
#ifdef IVW_SIGAR
    sigar_cpu_info_list_t cpulinfolist;
    int status = sigar_cpu_info_list_get(sigar_, &cpulinfolist);
    bool success = (status == SIGAR_OK);

    if (success) {
        infoCPUs_.resize(cpulinfolist.number);

        for (unsigned long i=0; i<cpulinfolist.number; i++) {
            sigar_cpu_info_t cpu_info = cpulinfolist.data[i];
            infoCPUs_[i].vendor = std::string(cpu_info.vendor);
            infoCPUs_[i].model = std::string(cpu_info.model);
            infoCPUs_[i].mhz = static_cast<glm::u64>(cpu_info.mhz);
        }
    }

    sigar_cpu_info_list_destroy(sigar_, &cpulinfolist);
    return success;
#else
    return false;
#endif
}

bool SystemCapabilities::lookupMemoryInfo() {
#ifdef IVW_SIGAR
    sigar_mem_t meminfo;
    if (sigar_mem_get(sigar_, &meminfo) == SIGAR_OK) {
        infoRAM_.total = util::megabytes_to_bytes(static_cast<glm::u64>(meminfo.ram));
        infoRAM_.available = static_cast<glm::u64>(meminfo.free);
        return true;
    } else {
        return false;
    }
#else
    return false;
#endif
}

bool SystemCapabilities::lookupDiskInfo() {
    infoDisks_.clear();
#ifdef IVW_SIGAR
    sigar_file_system_list_t diskinfolist;
    sigar_file_system_usage_t diskusageinfo;
    int status = sigar_file_system_list_get(sigar_, &diskinfolist);
    bool success = (status == SIGAR_OK);

    if (success) {
        for (unsigned long i=0; i<diskinfolist.number; i++) {
            sigar_file_system_t disk_info = diskinfolist.data[i];
            status = sigar_file_system_usage_get(sigar_, disk_info.dir_name, &diskusageinfo);

            if (status == SIGAR_OK) {
                DiskInfo currentDiskInfo;
                currentDiskInfo.diskType = std::string(disk_info.type_name);
                currentDiskInfo.diskType[0] = static_cast<char>(toupper(currentDiskInfo.diskType[0]));

                if (currentDiskInfo.diskType == "Local") {
                    currentDiskInfo.diskName = std::string(disk_info.dev_name);
                    currentDiskInfo.total = util::kilobytes_to_bytes(static_cast<glm::u64>(diskusageinfo.total));
                    currentDiskInfo.free = util::kilobytes_to_bytes(static_cast<glm::u64>(diskusageinfo.free));
                    infoDisks_.push_back(currentDiskInfo);
                }
            }
        }
    }

    sigar_file_system_list_destroy(sigar_, &diskinfolist);
    return success;
#else
    return false;
#endif
}

bool SystemCapabilities::lookupProcessMemoryInfo() {
#ifdef IVW_SIGAR
    sigar_proc_mem_t meminfo;

    if (sigar_proc_mem_get(sigar_, sigar_pid_get(sigar_), &meminfo) == SIGAR_OK) {
        infoProcRAM_.residentMem = static_cast<glm::u64>(meminfo.resident);
        infoProcRAM_.sharedMem = static_cast<glm::u64>(meminfo.share);
        infoProcRAM_.virtualMem = static_cast<glm::u64>(meminfo.size);
        return true;
    } else {
        return false;
    }
#else
    return false;
#endif
}

// helper struct to define arbitrary separator characters in a locale
// usage:
//   std::stringstream stream;
//   stream.imbue(std::locale(stream.getloc(), new CsvWhitespace));
struct IniSeparator : std::ctype<char> {
    static const mask* makeTable() {
        // copy table of C locale
        static std::vector<mask> m(classic_table(), classic_table() + table_size);
        m[' '] &= ~space; // remove space as whitespace
        m['='] |= space;
        return &m[0];
    }
    IniSeparator(std::size_t refs = 0) : ctype(makeTable(), false, refs) {}
};

void SystemCapabilities::readBuildInfoFromIni() {
    auto dir = InviwoApplication::getPtr()->getBinaryPath() + "/inviwo_buildinfo.ini";
    std::ifstream in(dir.c_str(), std::ios::in);
    if (!in.is_open()) {
        return;
    }

    buildInfo_ = BuildInfo();

    std::istringstream iss;
    iss.imbue(std::locale(iss.getloc(), new IniSeparator()));
    enum class Section { Unknown, Date, Hashes };

    std::string line;
    Section currentSection = Section::Unknown;
    while (std::getline(in, line)) {
        line = trim(line);
        // ignore comment, i.e. line starts with ';'
        if (line.empty() || line[0] == ';') {
            continue;
        }
        if (line == "[date]") {
            currentSection = Section::Date;
        }
        else if (line == "[hashes]") {
            currentSection = Section::Hashes;
        }
        else if (line[0] == '[') {
            currentSection = Section::Unknown;
        }
        else {
            // read in key value pairs
            iss.clear();
            iss.str(line);
            std::string key;
            std::string value;
            if (!(iss >> key >> value)) {
                // invalid key-value pair, ignore it
                continue;
            }
            switch (currentSection) {
                case Section::Date:
                {
                    int valuei = std::stoi(value);
                    if (key == "year") {
                        buildInfo_.year = valuei;
                    }
                    else if (key == "month") {
                        buildInfo_.month = valuei;
                    }
                    else if (key == "day") {
                        buildInfo_.day = valuei;
                    }
                    else if (key == "hour") {
                        buildInfo_.hour = valuei;
                    }
                    else if (key == "minute") {
                        buildInfo_.minute = valuei;
                    }
                    else if (key == "second") {
                        buildInfo_.second = valuei;
                    }
                    break;
                }
                case Section::Hashes:
                    buildInfo_.githashes.push_back({ key, value });
                    break;
                case Section::Unknown:
                default:
                    break;
            }
        }
    }
}

void SystemCapabilities::printInfo() {
    retrieveDynamicInfo();

    // Try to retrieve operating system information
    if (successOSInfo_) {
        LogInfoCustom("SystemInfo", "OS: " << infoOS_.description << " " << infoOS_.platform
                                           << "-bit");
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
                        LogInfoCustom("SystemInfo", "CPU " << i + 1 << "-" << i + 1 + count << ": "
                                                           << prevCPU);
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

glm::u64 SystemCapabilities::getAvailableMemory() {
    successMemoryInfo_ = lookupMemoryInfo();

    if (successMemoryInfo_) {
        return infoRAM_.available;
    } else {
        return 0;
    }
}

glm::u64 SystemCapabilities::getCurrentResidentMemoryUsage() {
    successProcessMemoryInfo_ = lookupProcessMemoryInfo();

    if (successMemoryInfo_) {
        return infoProcRAM_.residentMem;
    } else {
        return 0;
    }
}

} // namespace
