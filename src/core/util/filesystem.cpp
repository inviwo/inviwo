/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/inviwocommondefines.h>
#include <inviwo/core/util/safecstr.h>

// For directory exists
#include <sys/types.h>
#include <sys/stat.h>

// For working directory
#include <cstdio>  // FILENAME_MAX
#include <codecvt>
#include <cctype>  // isdigit()
#include <cerrno>

#ifdef WIN32
struct IUnknown;  // Workaround for "combaseapi.h(229): error C2187: syntax error: 'identifier' was
                  // unexpected here" when using /permissive-
#include <windows.h>
#include <tchar.h>
#include <direct.h>
#include <Shlobj.h>
#include <psapi.h>
#elif defined(__APPLE__)
#include <CoreServices/CoreServices.h>
#include <libproc.h>  // proc_pidpath
#include <unistd.h>
#include <fcntl.h>  // open
// sendfile
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <mach-o/dyld.h>

#include <sysdir.h>  // for sysdir_start_search_path_enumeration
#include <glob.h>    // for glob needed to expand ~ to user dir

#else
#include <unistd.h>
#include <fcntl.h>  // open
#include <sys/sendfile.h>
#include <dlfcn.h>
#include <elf.h>  // To retrieve rpath
#include <link.h>
#endif

#include <array>
#include <algorithm>
#include <string_view>
#include <chrono>

#include <fmt/format.h>
#include <fmt/std.h>

namespace fs = std::filesystem;

namespace inviwo {

namespace filesystem {

FILE* fopen(const fs::path& filename, const char* mode) {
#if defined(_WIN32)
    return _wfopen(filename.c_str(), util::toWstring(mode).c_str());
#else
    return ::fopen(filename.c_str(), mode);
#endif
}

std::fstream fstream(const fs::path& filename, std::ios_base::openmode mode) {
    return std::fstream(filename, mode);
}

std::ifstream ifstream(const fs::path& filename, std::ios_base::openmode mode) {
    return std::ifstream(filename, mode);
}

std::ofstream ofstream(const fs::path& filename, std::ios_base::openmode mode) {
    return std::ofstream(filename, mode);
}

bool skipByteOrderMark(std::istream& stream) {
    // Check presence of BOM and skip it if it does
    std::array<uint8_t, 3> dataToCheck{0, 0, 0};
    std::streampos streamPos = stream.tellg();
    stream.read(reinterpret_cast<char*>(dataToCheck.data()), sizeof(dataToCheck));
    auto bytesRead = stream.gcount();

    // UTF-8 byte order mark
    // https://en.wikipedia.org/wiki/Byte_order_mark
    std::array<uint8_t, 3> bom = {0xef, 0xbb, 0xbf};

    if (bytesRead == sizeof(dataToCheck) && dataToCheck == bom) {
        // Contains BOM, skip it since it is not part of the data
        return true;
    } else {
        // No BOM found, rewind to beginning of the stream
        stream.seekg(streamPos, std::ios::beg);
        return false;
    }
}

fs::path getWorkingDirectory() {
#ifdef WIN32
    auto bufferSize = GetCurrentDirectoryW(0, nullptr);
    std::wstring buff(bufferSize, 0);

    if (!GetCurrentDirectoryW(static_cast<DWORD>(buff.size()), buff.data()))
        throw Exception("Error querying current directory",
                        IVW_CONTEXT_CUSTOM("filesystem::getWorkingDirectory"));
    return fs::path{buff};
#else
    std::array<char, FILENAME_MAX> workingDir;
    if (!getcwd(workingDir.data(), workingDir.size()))
        throw Exception("Error querying current directory",
                        IVW_CONTEXT_CUSTOM("filesystem::getWorkingDirectory"));
    return fs::path(workingDir.data());
#endif
}

void setWorkingDirectory(const fs::path& path) {
#ifdef WIN32
    SetCurrentDirectoryW(path.c_str());
#else
    if (chdir(path.c_str()) != 0) {
        throw Exception(IVW_CONTEXT_CUSTOM("filesystem::setWorkingDirectory"),
                        "Error setting working directory path: (}", path);
    }
#endif
}

#ifdef WIN32
fs::path getModuleFileName(HMODULE handle, std::string_view name) {
    const DWORD maxBufSize = 1 << 20;  // corresponds to 1MiB
    std::wstring buffer(FILENAME_MAX, 0);

    auto size = GetModuleFileNameW(handle, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (size == 0) {
        throw Exception(IVW_CONTEXT_CUSTOM("filesystem::getModuleFileName"),
                        "Error retrieving {} path", name);
    }
    while (size == buffer.size()) {
        // buffer is too small, enlarge
        auto newSize = buffer.size() * 2;
        if (newSize > maxBufSize) {
            throw Exception(IVW_CONTEXT_CUSTOM("filesystem::getModuleFileName"),
                            "Insufficient buffer size");
        }
        buffer.resize(newSize, 0);
        size = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (size == 0) {
            throw Exception(IVW_CONTEXT_CUSTOM("filesystem::getModuleFileName"),
                            "Error retrieving {} path", name);
        }
    }
    buffer.resize(size);
    return fs::path{buffer};
}
#endif

fs::path getExecutablePath() {
    // http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe

#ifdef WIN32
    return getModuleFileName(nullptr, "executable");  // nullptr will lookup the exe module.

#elif __APPLE__
    // http://stackoverflow.com/questions/799679/programatically-retrieving-the-absolute-path-of-an-os-x-command-line-app/1024933#1024933
    std::array<char, PROC_PIDPATHINFO_MAXSIZE> executablePath;
    auto pid = getpid();
    if (proc_pidpath(pid, executablePath.data(), executablePath.size()) <= 0) {
        // Error retrieving path
        throw Exception("Error retrieving executable path",
                        IVW_CONTEXT_CUSTOM("filesystem::getExecutablePath"));
    }
    return std::string(executablePath.data());

#else  // Linux
    std::array<char, FILENAME_MAX> executablePath;
    auto size = ::readlink("/proc/self/exe", executablePath.data(), executablePath.size() - 1);
    if (size != -1) {
        // readlink does not append a NUL character to the path
        executablePath[size] = '\0';
    } else {
        // Error retrieving path
        throw Exception("Error retrieving executable path",
                        IVW_CONTEXT_CUSTOM("filesystem::getExecutablePath"));
    }
    return std::string(executablePath.data());
#endif
}

fs::path getInviwoBinDir() {
#ifdef WIN32
    auto handle = GetModuleHandleA("inviwo-core");

    if (!handle) {
        handle = GetModuleHandleA("inviwo-cored");
    }

    if (!handle) {
        throw Exception("Error retrieving inviwo-core path",
                        IVW_CONTEXT_CUSTOM("filesystem::getInviwoBinDir"));
    }

    // nullptr will lookup the exe module.
    return getModuleFileName(handle, "inviwo-core").parent_path();

#else
    auto allLibs = getLoadedLibraries();
    auto it = std::find_if(allLibs.begin(), allLibs.end(), [&](const auto& lib) {
        return lib.string().find("inviwo-core") != std::string::npos;
    });

    if (it != allLibs.end()) {
        return it->parent_path();
    } else {
        throw Exception("Error retrieving inviwo-core path",
                        IVW_CONTEXT_CUSTOM("filesystem::getInviwoBinDir"));
    }
#endif
}

#if defined(__APPLE__)

std::string expandTilde(const char* str) {
    if (!str) return {};

    glob_t globbuf;
    if (glob(str, GLOB_TILDE, nullptr, &globbuf) == 0) {  // success
        std::string result(globbuf.gl_pathv[0]);
        globfree(&globbuf);
        return result;
    } else {
        throw Exception(IVW_CONTEXT_CUSTOM("filesystem"), "Unable to expand tilde in string '{}'",
                        str);
    }
}

#endif

fs::path getInviwoUserSettingsPath() {
    std::stringstream ss;
#ifdef _WIN32
    PWSTR path;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path);
    if (SUCCEEDED(hr)) {
        const std::wstring wpath(path);
        CoTaskMemFree(path);
        ss << util::fromWstring(wpath) << "/Inviwo";
    } else {
        throw Exception("Failed to get settings folder", IVW_CONTEXT_CUSTOM("filesystem"));
    }

#elif defined(__unix__)
    ss << std::getenv("HOME");
    ss << "/.inviwo";

#elif defined(__APPLE__)
    char path[PATH_MAX];
    auto state = sysdir_start_search_path_enumeration(SYSDIR_DIRECTORY_APPLICATION_SUPPORT,
                                                      SYSDIR_DOMAIN_MASK_USER);
    if ((state = sysdir_get_next_search_path_enumeration(state, path))) {
        ss << expandTilde(path) << "/org.inviwo.network-editor";
    } else {
        throw Exception("Failed to get settings folder", IVW_CONTEXT_CUSTOM("filesystem"));
    }

#else
    LogWarnCustom("filesystem::getInviwoApplicationPath",
                  "Get User Setting Path is not implemented for current system");
#endif
    return ss.str();
}

bool fileExists(const fs::path& filePath) { return fs::is_regular_file(filePath); }

bool directoryExists(const fs::path& path) { return fs::is_directory(path); }

std::time_t fileModificationTime(const fs::path& filePath) {
    const auto fsTime = fs::last_write_time(filePath);
    const auto systemTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        fsTime - std::chrono::file_clock::now() + std::chrono::system_clock::now());
    return std::chrono::system_clock::to_time_t(systemTime);
}

bool copyFile(const fs::path& src, const fs::path& dst) {
    std::error_code ec;
    return fs::copy_file(src, dst, ec);
}

std::vector<fs::path> getDirectoryContents(const fs::path& path, ListMode mode) {
    if (!fs::is_directory(path)) {
        return {};
    }

    std::vector<fs::path> res;

    auto begin = std::filesystem::directory_iterator{path};
    auto end = std::filesystem::directory_iterator{};

    switch (mode) {
        case ListMode::Files:
            std::copy_if(begin, end, std::back_inserter(res),
                         [](const fs::directory_entry& item) { return item.is_regular_file(); });
            break;
        case ListMode::Directories:
            std::copy_if(begin, end, std::back_inserter(res),
                         [](const fs::directory_entry& item) { return item.is_directory(); });
            break;
        case ListMode::FilesAndDirectories:
            std::copy_if(begin, end, std::back_inserter(res), [](const fs::directory_entry& item) {
                return item.is_regular_file() || item.is_directory();
            });
            break;
        default:
            std::copy_if(begin, end, std::back_inserter(res),
                         [](const fs::directory_entry& item) { return item.is_regular_file(); });
            break;
    }

    std::transform(res.begin(), res.end(), res.begin(),
                   [&](const fs::path& child) { return fs::relative(child, path); });

    std::sort(res.begin(), res.end());

    return res;
}

std::vector<fs::path> getDirectoryContentsRecursively(const fs::path& path, ListMode mode) {
    if (!fs::is_directory(path)) {
        return {};
    }

    std::vector<fs::path> res;

    auto begin = std::filesystem::recursive_directory_iterator{path};
    auto end = std::filesystem::recursive_directory_iterator{};

    switch (mode) {
        case ListMode::Files:
            std::copy_if(begin, end, std::back_inserter(res),
                         [](const fs::directory_entry& item) { return item.is_regular_file(); });
            break;
        case ListMode::Directories:
            std::copy_if(begin, end, std::back_inserter(res),
                         [](const fs::directory_entry& item) { return item.is_directory(); });
            break;
        case ListMode::FilesAndDirectories:
            std::copy_if(begin, end, std::back_inserter(res), [](const fs::directory_entry& item) {
                return item.is_regular_file() || item.is_directory();
            });
            break;
        default:
            std::copy_if(begin, end, std::back_inserter(res),
                         [](const fs::directory_entry& item) { return item.is_regular_file(); });
            break;
    }

    std::transform(res.begin(), res.end(), res.begin(),
                   [&](const fs::path& child) { return fs::relative(child, path); });

    std::sort(res.begin(), res.end());

    return res;
}

bool wildcardStringMatch(const std::string& pattern, const std::string& str) {
    const char* patternPtr = pattern.c_str();
    const char* strPtr = str.c_str();

    const char* patternPtrSave = nullptr;
    const char* strPtrSave = nullptr;

    while (*strPtr != '\0') {
        if (*patternPtr == '*') {
            // wildcard detected
            ++patternPtr;  // remove wildcard from pattern
            if (*patternPtr == '\0') {
                // wildcard at the end of the pattern matches remaining string
                return true;
            }
            patternPtrSave = patternPtr;  // save position after wildcard for roll-back
            strPtrSave = strPtr + 1;
        } else if ((*patternPtr == '?') || (*patternPtr == *strPtr)) {
            // single character match or single character wildcard
            ++patternPtr;
            ++strPtr;
        } else if (!strPtrSave) {
            // early exit if the next string character is at the end
            return false;
        } else {
            // roll-back in case read for next character after wildcard was not successful
            patternPtr = patternPtrSave;
            // gobble up current character and consider it being part of the wildcard
            strPtr = strPtrSave++;
        }
    }
    // account for wildcards at the end of the pattern
    while (*patternPtr == '*') {
        ++patternPtr;
    }
    // pattern only matches if nothing of it is left
    return (*patternPtr == '\0');
}

bool wildcardStringMatchDigits(const std::string& pattern, const std::string& str, int& index,
                               bool matchLess, bool matchMore) {

    const char* patternPtr = pattern.c_str();
    const char* strPtr = str.c_str();

    const char* patternPtrSave = nullptr;
    const char* strPtrSave = nullptr;

    int result = 0;

    while (*strPtr != '\0') {
        if ((*patternPtr == '#') && (isdigit(*strPtr))) {
            // digit detected
            result = result * 10 + static_cast<int>(*strPtr - '0');

            ++patternPtr;
            ++strPtr;

            if (matchMore) {
                // more digits (eat up all digits)
                while ((*strPtr != '\0') && (*patternPtr != '#') && (isdigit(*strPtr))) {
                    result = result * 10 + static_cast<int>(*strPtr - '0');
                    ++strPtr;
                }
            }
            if (matchLess) {
                // less digits (eat up all '#')
                while ((*patternPtr != '\0') && (*patternPtr == '#') && (!isdigit(*strPtr))) {
                    ++patternPtr;
                }
            }
            if (*patternPtr != '#' && isdigit(*strPtr)) {
                // run out of '#' but there are still digits left
                strPtrSave = nullptr;  // reset wildcard mode for regular pattern matching
            }
        } else if (*patternPtr == '*') {
            // wildcard detected
            ++patternPtr;  // remove wildcard from pattern
            if (*patternPtr == '\0') {
                // wildcard at the end of the pattern matches remaining string
                index = result;
                return true;
            }
            patternPtrSave = patternPtr;  // save position after wildcard for roll-back
            strPtrSave = strPtr + 1;
        } else if ((*patternPtr == '?') || (*patternPtr == *strPtr)) {
            // single character match or single character wildcard
            ++patternPtr;
            ++strPtr;
        } else if (!strPtrSave) {
            // early exit if the next string character is at the end
            return false;
        } else {
            // roll-back in case read for next character after wildcard was not successful
            patternPtr = patternPtrSave;
            // gobble up current character and consider it being part of the wildcard
            strPtr = strPtrSave++;
        }
    }
    // account for wildcards at the end of the pattern
    while (*patternPtr == '*') {
        ++patternPtr;
    }
    // pattern matches only if nothing of it is left
    if (*patternPtr == '\0') {
        index = result;
        return true;
    }

    return false;
}

std::optional<fs::path> getParentFolderWithChildren(const fs::path& path,
                                                    std::span<const fs::path> childFolders) {
    auto currentDir = path;

    fs::path prevDir;
    do {
        if (std::all_of(childFolders.begin(), childFolders.end(), [&](const fs::path& childFolder) {
                return fs::is_directory(currentDir / childFolder);
            })) {
            return currentDir;
        }

        prevDir = currentDir;
        currentDir = currentDir.parent_path();
    } while (prevDir != currentDir);

    // no matching parent folder found
    return std::nullopt;
}

fs::path findBasePath() {
#if defined(__APPLE__)
    // We might be in a bundle
    // the executable will be in
    //    Inviwo.app/Contents/MacOS/Inviwo
    // and the our base should be
    //    Inviwo.app/Contents/Resources

    CFURLRef appUrlRef =
        CFBundleCopyResourceURL(CFBundleGetMainBundle(), CFSTR("data"), NULL, NULL);
    if (appUrlRef) {
        CFStringRef filePathRef = CFURLCopyPath(appUrlRef);
        const char* filePath = CFStringGetCStringPtr(filePathRef, kCFStringEncodingUTF8);
        fs::path macPath(filePath);
        // Release references
        CFRelease(filePathRef);
        CFRelease(appUrlRef);

        if (fs::is_directory(macPath)) {
            // remove "data"
            return macPath.parent_path();
        }
    }
#endif
    // Search process:
    // Modules folder might exist during development, so first try with
    // both data/workspaces and modules folder.
    // If they are not found we might be running through a debugger, so try source directory.
    // If neither of above works then we probably have an application without a data/workspaces
    // folder, so become less restrictive and only search for the modules folder. If nothing works
    // then use the executable path, but warn that this might have negative effects.

    // locate Inviwo base path matching the subfolders data/workspaces and modules

    std::array children = {fs::path{"modules"}, fs::path{"data/workspaces"}};

    if (auto path = getParentFolderWithChildren(getExecutablePath(), children)) {
        return *path;
    }

#ifdef INVIWO_ALL_DYN_LINK
    // If we have linking dynamically use getInviwoBinDir() which looks for inviwo-core
    if (auto path = getParentFolderWithChildren(getInviwoBinDir(), children)) {
        return *path;
    }
#endif

    // could not locate base path relative to executable or bin dir, try CMake source path
    if (fs::is_directory(fs::path{build::sourceDirectory} / "data" / "workspaces") &&
        fs::is_directory(fs::path{build::sourceDirectory} / "modules")) {
        return std::string{build::sourceDirectory};
    }

    // Relax the criterion, only require the modules folder
    if (auto path =
            getParentFolderWithChildren(getExecutablePath(), std::span(children.begin(), 1))) {
        return *path;
    }

#ifdef INVIWO_ALL_DYN_LINK
    if (auto path =
            getParentFolderWithChildren(getInviwoBinDir(), std::span(children.begin(), 1))) {
        return *path;
    }
#endif

    LogErrorCustom(
        "filesystem::findBasePath",
        "Could not locate Inviwo base path meaning that application data might not be found.");
    return getExecutablePath();
}

fs::path getPath(PathType pathType, const std::string& suffix, const bool createFolder) {
    auto result = findBasePath();

    switch (pathType) {
        case PathType::Data:
            result /= "data";
            break;

        case PathType::Volumes:
            result /= "data/volumes";
            break;

        case PathType::Workspaces:
            result /= "data/workspaces";
            break;

        case PathType::PortInspectors:
            result /= "data/workspaces/portinspectors";
            break;

        case PathType::Scripts:
            result /= "data/scripts";
            break;

        case PathType::Images:
            result /= "data/images";
            break;

        case PathType::Databases:
            result /= "data/databases";
            break;

        case PathType::Resources:
            result /= "resources";
            break;

        case PathType::TransferFunctions:
            result /= "data/transferfunctions";
            break;

        case PathType::Settings:
            result = getInviwoUserSettingsPath();
            break;
        case PathType::Modules:
            result = getInviwoUserSettingsPath() / "modules";
            break;
        case PathType::Help:
            result /= "data/help";
            break;

        case PathType::Tests:
            result /= "tests";
            break;

        default:
            break;
    }

    if (createFolder) {
        fs::create_directories(result);
    }
    result += suffix;
    return result.lexically_normal();
}

void createDirectoryRecursively(const fs::path& path) { fs::create_directories(path); }

fs::path addBasePath(const fs::path& url) { return findBasePath() / url; }

fs::path getFileDirectory(const fs::path& url) { return url.parent_path(); }

fs::path getFileNameWithExtension(const fs::path& url) { return url.filename(); }

fs::path getFileNameWithoutExtension(const fs::path& url) { return url.stem(); }

std::string getFileExtension(const fs::path& url) {
    auto ext = url.extension().string();
    if (ext.size() >= 1 && ext[0] == '.') {
        return ext.substr(1);
    } else {
        return ext;
    }
}

fs::path replaceFileExtension(const fs::path& url, std::string_view newFileExtension) {
    auto result = url;
    return result.replace_extension(newFileExtension);
}

fs::path getRelativePath(const fs::path& basePath, const fs::path& absolutePath) {
    return fs::relative(absolutePath, basePath);
}

fs::path getCanonicalPath(const fs::path& url) { return fs::weakly_canonical(url); }

bool isAbsolutePath(const fs::path& path) { return path.is_absolute(); }

bool sameDrive(const fs::path& refPath, const fs::path& queryPath) {
    return refPath.root_name() == queryPath.root_name();
}

fs::path cleanupPath(std::string_view path) {
    if (path.empty()) {
        return {};
    }

    std::string result(path);
    // replace backslashes '\' with forward slashes '/'
    std::replace(result.begin(), result.end(), '\\', '/');

    // check for matching quotes at begin and end
    if ((result.size() > 1) && (result.front() == '\"') && (result.back() == '\"')) {
        result = result.substr(1, result.size() - 2);
    }
    if (result.size() > 2) {
        // ensure that drive letter is an uppercase character, but there might be an unmatched quote
        const size_t driveLetter = (result[0] == '\"') ? 1 : 0;
        if (result[driveLetter + 1] == ':') {
            result[driveLetter] = static_cast<char>(toupper(result[driveLetter]));
        }
    }

    return result;
}

int getCurrentProcessId() {
#ifdef WIN32
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}

#if WIN32

std::vector<fs::path> getLoadedLibraries() {
    std::vector<fs::path> res;

    DWORD processID = GetCurrentProcessId();
    HMODULE hMods[1024];
    HANDLE hProcess;
    DWORD cbNeeded;
    unsigned int i;

    // Get a handle to the process.
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (NULL == hProcess) return {};

    // Get a list of all the modules in this process.
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            TCHAR szModName[MAX_PATH];
            // Get the full path to the module's file.
            if (GetModuleFileNameExW(hProcess, hMods[i], szModName,
                                     sizeof(szModName) / sizeof(TCHAR))) {

                std::string name = util::fromWstring(std::wstring(szModName));
                replaceInString(name, "\\", "/");
                res.emplace_back(name);
            }
        }
    }

    // Release the handle to the process.
    CloseHandle(hProcess);

    return res;
}

#elif defined(__APPLE__)

std::vector<fs::path> getLoadedLibraries() {
    std::vector<fs::path> res;
    const uint32_t count = _dyld_image_count();
    for (uint32_t i = 0; i < count; i++) {
        res.emplace_back(_dyld_get_image_name(i));
    }
    return res;
}

#else

namespace {
int visitLibraries(struct dl_phdr_info* info, size_t size, void* data) {
    auto res = reinterpret_cast<std::vector<fs::path>*>(data);
    res->emplace_back(info->dlpi_name);
    return 0;
}
}  // namespace
std::vector<fs::path> getLoadedLibraries() {
    std::vector<fs::path> res;
    dl_iterate_phdr(visitLibraries, reinterpret_cast<void*>(&res));
    return res;
}

#endif

}  // namespace filesystem

}  // end namespace inviwo
