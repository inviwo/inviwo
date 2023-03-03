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
#include <inviwo/core/util/tinydirinterface.h>
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

#include <fmt/format.h>

namespace inviwo {

namespace detail {

// If path contains the location of a directory, it cannot contain a trailing backslash.
// If it does, stat will return -1 and errno will be set to ENOENT.
// https://msdn.microsoft.com/en-us/library/14h5k7ff.aspx
std::string_view removeTrailingSlash(std::string_view path) {
    // Remove trailing backslash or slash
    if (path.size() > 1 && (path.back() == '/' || path.back() == '\\')) {
        return path.substr(0, path.size() - 1);
    }
    return path;
}

}  // namespace detail

namespace filesystem {

FILE* fopen(std::string_view filename, const char* mode) {
#if defined(_WIN32)
    return _wfopen(util::toWstring(filename).c_str(), util::toWstring(mode).c_str());
#else
    return ::fopen(SafeCStr{filename}.c_str(), mode);
#endif
}

std::fstream fstream(std::string_view filename, std::ios_base::openmode mode) {
#if defined(_WIN32)
    return std::fstream(util::toWstring(filename), mode);
#else
    return std::fstream(SafeCStr{filename}.c_str(), mode);
#endif
}

std::ifstream ifstream(std::string_view filename, std::ios_base::openmode mode) {
#if defined(_WIN32)
    return std::ifstream(util::toWstring(filename), mode);
#else
    return std::ifstream(SafeCStr{filename}.c_str(), mode);
#endif
}

std::ofstream ofstream(std::string_view filename, std::ios_base::openmode mode) {
#if defined(_WIN32)
    return std::ofstream(util::toWstring(filename), mode);
#else
    return std::ofstream(SafeCStr{filename}.c_str(), mode);
#endif
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

std::string getWorkingDirectory() {
#ifdef WIN32
    auto bufferSize = GetCurrentDirectoryW(0, nullptr);
    std::wstring buff(bufferSize, 0);

    if (!GetCurrentDirectoryW(static_cast<DWORD>(buff.size()), buff.data()))
        throw Exception("Error querying current directory",
                        IVW_CONTEXT_CUSTOM("filesystem::getWorkingDirectory"));
    return cleanupPath(util::fromWstring(buff));
#else
    std::array<char, FILENAME_MAX> workingDir;
    if (!getcwd(workingDir.data(), workingDir.size()))
        throw Exception("Error querying current directory",
                        IVW_CONTEXT_CUSTOM("filesystem::getWorkingDirectory"));
    return cleanupPath(std::string(workingDir.data()));
#endif
}

void setWorkingDirectory(std::string_view path) {
#ifdef WIN32
    auto wPath = util::toWstring(path);
    SetCurrentDirectoryW(wPath.c_str());
#else
    SafeCStr cPath{path};
    chdir(cPath.c_str());
#endif
}

#ifdef WIN32
std::string getModuleFileName(HMODULE handle, std::string_view name) {
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
    return util::fromWstring(buffer);
}
#endif

std::string getExecutablePath() {
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

std::string getInviwoBinDir() {
#ifdef WIN32
    auto handle = GetModuleHandleA("inviwo-core");

    if (!handle) {
        handle = GetModuleHandleA("inviwo-cored");
    }

    if (!handle) {
        throw Exception("Error retrieving inviwo-core path",
                        IVW_CONTEXT_CUSTOM("filesystem::getInviwoBinDir"));
    }

    return getFileDirectory(
        getModuleFileName(handle, "inviwo-core"));  // nullptr will lookup the exe module.

#else
    auto allLibs = getLoadedLibraries();
    auto it = std::find_if(allLibs.begin(), allLibs.end(), [&](const auto& lib) {
        return lib.find("inviwo-core") != std::string::npos;
    });

    if (it != allLibs.end()) {
        return getFileDirectory(*it);
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

std::string getInviwoUserSettingsPath() {
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

bool fileExists(std::string_view filePath) {
// http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
#if defined(_WIN32)
    struct _stat64 buffer;
    return (_wstat64(util::toWstring(filePath).c_str(), &buffer) == 0);
#else
    struct stat buffer;
    return (stat(SafeCStr<256>{filePath}.c_str(), &buffer) == 0);
#endif
}

bool directoryExists(std::string_view path) {
// If path contains the location of a directory, it cannot contain a trailing backslash.
// If it does, -1 will be returned and errno will be set to ENOENT.
// https://msdn.microsoft.com/en-us/library/14h5k7ff.aspx
// We therefore check if path ends with a backslash
#if defined(_WIN32)
    struct _stat buffer;
    int retVal = _wstat(util::toWstring(detail::removeTrailingSlash(path)).c_str(), &buffer);
    return (retVal == 0 && (buffer.st_mode & S_IFDIR));
#else
    struct stat buffer;
    return (stat(SafeCStr<256>{detail::removeTrailingSlash(path)}.c_str(), &buffer) == 0 &&
            (buffer.st_mode & S_IFDIR));
#endif
}

std::time_t fileModificationTime(std::string_view filePath) {
    // If path contains the location of a directory, it cannot contain a trailing backslash.
    // If it does, -1 will be returned and errno will be set to ENOENT.
    // https://msdn.microsoft.com/en-us/library/14h5k7ff.aspx
    // We therefore check if path ends with a backslash
    auto err = 0;
#if defined(_WIN32)
    struct _stat buffer;
    err = _wstat(util::toWstring(detail::removeTrailingSlash(filePath)).c_str(), &buffer);
#else
    struct stat buffer;
    err = stat(SafeCStr<256>{detail::removeTrailingSlash(filePath)}.c_str(), &buffer);
#endif
    if (err != -1) {
        return buffer.st_mtime;
    } else {
        return 0;
    }
}

bool copyFile(std::string_view src_view, std::string_view dst_view) {
#ifdef WIN32
    // Copy file and overwrite if it exists.
    // != 0 to get rid of bool comparison warning (C4800)
    return CopyFileW(util::toWstring(src_view).c_str(), util::toWstring(dst_view).c_str(), FALSE) !=
           0;
#else
    SafeCStr<256> src{src_view};
    SafeCStr<256> dst{dst_view};

    int source = open(src.c_str(), O_RDONLY, 0);
    if (source < 0) {
        return false;
    }

    int dest = open(dst.c_str(), O_WRONLY | O_CREAT, 0644);
    if (dest < 0) {
        close(source);
        return false;
    }

    bool successful = false;
#if defined(__APPLE__)
    off_t bytesWritten = 0;  // send until the end of file has been reached
    successful = sendfile(dest, source, 0, &bytesWritten, nullptr, 0) == 0;
#else
    struct stat stat_source;
    fstat(source, &stat_source);
    auto bytesWritten = sendfile(dest, source, 0, stat_source.st_size);
    successful = bytesWritten > 0;
#endif
    close(source);
    close(dest);
    return successful;
#endif
}

std::vector<std::string> getDirectoryContents(const std::string& path, ListMode mode) {
    if (path.empty()) {
        return {};
    }
    if (!directoryExists(path)) {
        return {};
    }
    const auto tdmode = [&]() {
        switch (mode) {
            case ListMode::Files:
                return TinyDirInterface::ListMode::FilesOnly;
            case ListMode::Directories:
                return TinyDirInterface::ListMode::DirectoriesOnly;
            case ListMode::FilesAndDirectories:
                return TinyDirInterface::ListMode::FilesAndDirectories;
            default:
                return TinyDirInterface::ListMode::FilesOnly;
        }
    }();
    TinyDirInterface tinydir(path, tdmode);
    return tinydir.getContents();
}

std::vector<std::string> getDirectoryContentsRecursively(const std::string& path,
                                                         ListMode mode /*= ListMode::Files*/) {
    if (!directoryExists(path)) {
        return {};
    }

    auto content = filesystem::getDirectoryContents(path, mode);
    auto directories = filesystem::getDirectoryContents(path, filesystem::ListMode::Directories);
    if (mode == ListMode::Directories || mode == ListMode::FilesAndDirectories) {
        // Remove . and ..
        std::erase_if(content, [](const auto& dir) {
            return dir.compare(".") == 0 || dir.compare("..") == 0;
        });
    }

    for (auto& file : content) {
        file = path + "/" + file;
    }
    // Remove . and ..
    std::erase_if(directories, [](const auto& dir) {
        return dir.compare(".") == 0 || dir.compare("..") == 0;
    });

    for (auto& dir : directories) {
        dir = path + "/" + dir;
    }
    for (auto& dir : directories) {
        auto directoryContent = getDirectoryContentsRecursively(dir, mode);
        content.insert(content.end(), std::make_move_iterator(directoryContent.begin()),
                       std::make_move_iterator(directoryContent.end()));
    }
    return content;
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

std::optional<std::string> getParentFolderWithChildren(
    std::string_view path, const std::vector<std::string>& childFolders) {
    std::string currentDir = cleanupPath(path);
    size_t pos = currentDir.length();

    do {
        currentDir = currentDir.substr(0, pos);
        bool matchChildFolders = true;
        // check current folder whether it contains all requested child folders
        for (auto it = childFolders.begin(); it != childFolders.end() && matchChildFolders; ++it) {
            matchChildFolders = directoryExists(currentDir + "/" + *it);
        }
        if (matchChildFolders) return currentDir;
    } while ((pos = currentDir.rfind('/')) != std::string::npos);

    // no matching parent folder found
    return {};
}

std::string findBasePath() {
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
        std::string macPath(filePath);
        // Release references
        CFRelease(filePathRef);
        CFRelease(appUrlRef);

        if (directoryExists(macPath)) {
            // remove "data"
            return macPath.substr(0, macPath.rfind("/data"));
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

    if (auto path =
            getParentFolderWithChildren(getExecutablePath(), {"data/workspaces", "modules"})) {
        return *path;
    }

#ifdef INVIWO_ALL_DYN_LINK
    // If we have linking dynamically use getInviwoBinDir() which looks for inviwo-core
    if (auto path =
            getParentFolderWithChildren(getInviwoBinDir(), {"data/workspaces", "modules"})) {
        return *path;
    }
#endif

    // could not locate base path relative to executable or bin dir, try CMake source path
    if (directoryExists(fmt::format("{}/{}", build::sourceDirectory, "data/workspaces")) &&
        directoryExists(fmt::format("{}/{}", build::sourceDirectory, "modules"))) {
        return std::string{build::sourceDirectory};
    }

    // Relax the criterion, only require the modules folder
    if (auto path = getParentFolderWithChildren(getExecutablePath(), {"modules"})) {
        return *path;
    }

#ifdef INVIWO_ALL_DYN_LINK
    if (auto path = getParentFolderWithChildren(getInviwoBinDir(), {"modules"})) {
        return *path;
    }
#endif

    LogErrorCustom(
        "filesystem::findBasePath",
        "Could not locate Inviwo base path meaning that application data might not be found.");
    return getExecutablePath();
}

std::string getPath(PathType pathType, const std::string& suffix, const bool createFolder) {
    std::string result = findBasePath();

    switch (pathType) {
        case PathType::Data:
            result += "/data";
            break;

        case PathType::Volumes:
            result += "/data/volumes";
            break;

        case PathType::Workspaces:
            result += "/data/workspaces";
            break;

        case PathType::PortInspectors:
            result += "/data/workspaces/portinspectors";
            break;

        case PathType::Scripts:
            result += "/data/scripts";
            break;

        case PathType::Images:
            result += "/data/images";
            break;

        case PathType::Databases:
            result += "/data/databases";
            break;

        case PathType::Resources:
            result += "/resources";
            break;

        case PathType::TransferFunctions:
            result += "/data/transferfunctions";
            break;

        case PathType::Settings:
            result = getInviwoUserSettingsPath();
            break;
        case PathType::Modules:
            result = getInviwoUserSettingsPath() + "/modules";
            break;
        case PathType::Help:
            result += "/data/help";
            break;

        case PathType::Tests:
            result += "/tests";
            break;

        default:
            break;
    }

    if (createFolder) {
        createDirectoryRecursively(result);
    }
    return result + suffix;
}

void createDirectoryRecursively(std::string_view pathView) {
    auto path = cleanupPath(pathView);
    std::vector<std::string> v = util::splitString(path, '/');

    std::string pathPart;
#ifdef _WIN32
    pathPart += v.front();
    v.erase(v.begin());
#endif

    while (!v.empty()) {
        pathPart += "/" + v.front();
        v.erase(v.begin());
#ifdef _WIN32
        const auto wpart = util::toWstring(pathPart);
        if (_wmkdir(wpart.c_str()) != 0) {
            if (errno != EEXIST && errno != EISDIR) {
                throw Exception("Unable to create directory " + path,
                                IVW_CONTEXT_CUSTOM("filesystem"));
            }
        }
#elif defined(__unix__)
        if (mkdir(pathPart.c_str(), 0755) != 0) {
            if (errno != EEXIST && errno != EISDIR) {
                throw Exception("Unable to create directory " + path,
                                IVW_CONTEXT_CUSTOM("filesystem"));
            }
        }
#elif defined(__APPLE__)
        if (mkdir(pathPart.c_str(), 0755) != 0) {
            if (errno != EEXIST && errno != EISDIR) {
                throw Exception("Unable to create directory " + path,
                                IVW_CONTEXT_CUSTOM("filesystem"));
            }
        }
#else
        throw Exception("createDirectoryRecursively is not implemented for current system",
                        IVW_CONTEXT_CUSTOM("filesystem"));
#endif
    }
}  // namespace filesystem

std::string addBasePath(std::string_view url) {
    if (url.empty()) return findBasePath();
    return fmt::format("{}/{}", findBasePath(), url);
}

std::string getFileDirectory(std::string_view url) {
    std::string path = cleanupPath(url);
    size_t pos = path.rfind('/');
    if (pos == std::string::npos) return "";
    std::string fileDirectory = path.substr(0, pos);
    return fileDirectory;
}

std::string getFileNameWithExtension(std::string_view url) {
    std::string path = cleanupPath(url);
    size_t pos = path.rfind("/") + 1;
    // This relies on the fact that std::string::npos + 1 = 0
    std::string fileNameWithExtension = path.substr(pos, path.length());
    return fileNameWithExtension;
}

std::string getFileNameWithoutExtension(std::string_view url) {
    std::string fileNameWithExtension = getFileNameWithExtension(url);
    size_t pos = fileNameWithExtension.find_last_of(".");
    std::string fileNameWithoutExtension = fileNameWithExtension.substr(0, pos);
    return fileNameWithoutExtension;
}

std::string getFileExtension(std::string_view url) {
    std::string filename = getFileNameWithExtension(url);
    size_t pos = filename.rfind('.');

    if (pos == std::string::npos) return "";

    std::string fileExtension = filename.substr(pos + 1, url.length());
    return fileExtension;
}

std::string replaceFileExtension(std::string_view url, std::string_view newFileExtension) {
    size_t pos = url.rfind('.');
    return fmt::format("{}.{}", url.substr(0, pos), newFileExtension);
}

std::string getRelativePath(std::string_view basePath, std::string_view absolutePath) {
    const std::string absPath(getFileDirectory(cleanupPath(absolutePath)));
    const std::string fileName(getFileNameWithExtension(cleanupPath(absolutePath)));

    // path as string tokens
    auto basePathTokens = util::splitStringView(basePath, '/');
    auto absolutePathTokens = util::splitStringView(absPath, '/');

    size_t sizediff = 0;
    if (basePathTokens.size() < absolutePathTokens.size()) {
        sizediff = absolutePathTokens.size() - basePathTokens.size();
    }
    auto start = std::mismatch(absolutePathTokens.begin(), absolutePathTokens.end() - sizediff,
                               basePathTokens.begin(), basePathTokens.end());

    // add one ".." for each unique folder in basePathTokens
    std::vector<std::string> relativePath(std::distance(start.second, basePathTokens.end()), "..");

    // add append the unique folders in absolutePathTokens
    std::for_each(start.first, absolutePathTokens.end(),
                  [&](auto view) { relativePath.emplace_back(view); });

    if (!fileName.empty()) {
        relativePath.push_back(fileName);
    }

    return joinString(relativePath, "/");
}

std::string getCanonicalPath(const std::string& url) {
#ifdef WIN32
    const DWORD buffSize = 4096;  // MAX_PATH
    std::wstring urlWStr = util::toWstring(url);
    std::string result{url};

    WCHAR buffer[buffSize + 1];

    DWORD retVal = GetFullPathName(urlWStr.c_str(), buffSize, buffer, nullptr);
    if (retVal == 0) {
        // something went wrong, call GetLastError() to get the error code
        return result;
    } else if (retVal > buffSize) {
        // canonical path would be longer than buffer
        return result;
    } else {
        std::wstring resultWStr{buffer};
        result = util::fromWstring(resultWStr);
    }

    return result;
#else
#ifndef PATH_MAX
    const int PATH_MAX = 4096;
#endif
    char buffer[PATH_MAX + 1];
    char* retVal = realpath(url.c_str(), buffer);
    if (retVal == nullptr) {
        // something went wrong, check errno for error
        return url;
    } else {
        return std::string{retVal};
    }
#endif
}

bool isAbsolutePath(const std::string& path) {
#ifdef WIN32
    if (path.empty()) return false;

    // check for '[A-Z]:' in the begin of path, which might be quoted
    std::string_view str{path};
    if (str[0] == '\"') {
        str.remove_prefix(1);
    }
    if (str.length() < 2) return false;

    char driveLetter = static_cast<char>(toupper(str[0]));
    return ((driveLetter >= 'A') && (driveLetter <= 'Z') && (str[1] == ':'));

#else
    if (path.empty()) return false;

    return (path[0] == '/');

#endif
}

bool sameDrive(const std::string& refPath, const std::string& queryPath) {
#ifdef WIN32
    bool refPathIsRelative = !isAbsolutePath(refPath);
    bool queryPathIsRelative = !isAbsolutePath(queryPath);
    std::string referencePath(refPath);  // local copy of refPath

    if (refPathIsRelative) {
        if (queryPathIsRelative) {
            // both paths are relative, assuming same drive
            return true;
        } else {
            // reference path is relative, but queryPath is absolute
            // use base path as reference
            referencePath = findBasePath();
        }
    } else if (queryPathIsRelative) {
        // refPath is absolute, queryPath is relative
        return true;
    }

    if (referencePath.empty() || queryPath.empty()) return false;

    // check equality of drive letters
    return (toupper(referencePath[0]) == toupper(queryPath[0]));

#else
    return true;
#endif
}

std::string cleanupPath(std::string_view path) {
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

#if WIN32

std::vector<std::string> getLoadedLibraries() {
    std::vector<std::string> res;

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
                res.push_back(name);
            }
        }
    }

    // Release the handle to the process.
    CloseHandle(hProcess);

    return res;
}

#elif defined(__APPLE__)

std::vector<std::string> getLoadedLibraries() {
    std::vector<std::string> res;
    const uint32_t count = _dyld_image_count();
    for (uint32_t i = 0; i < count; i++) {
        res.emplace_back(_dyld_get_image_name(i));
    }
    return res;
}

#else

namespace {
int visitLibraries(struct dl_phdr_info* info, size_t size, void* data) {
    auto res = reinterpret_cast<std::vector<std::string>*>(data);
    res->emplace_back(info->dlpi_name);
    return 0;
}
}  // namespace
std::vector<std::string> getLoadedLibraries() {
    std::vector<std::string> res;
    dl_iterate_phdr(visitLibraries, reinterpret_cast<void*>(&res));
    return res;
}

#endif

}  // namespace filesystem

}  // end namespace inviwo
