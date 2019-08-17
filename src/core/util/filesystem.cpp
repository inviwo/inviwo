/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

// For directory exists
#include <sys/types.h>
#include <sys/stat.h>

// For working directory
#include <stdio.h>  // FILENAME_MAX
#include <codecvt>
#include <cctype>  // isdigit()

#ifdef WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
struct IUnknown;  // Workaround for "combaseapi.h(229): error C2187: syntax error: 'identifier' was
                  // unexpected here" when using /permissive-
#include <windows.h>
#include <tchar.h>
#include <direct.h>
#include <Shlobj.h>
#elif defined(__APPLE__)
#include <CoreServices/CoreServices.h>
#include <libproc.h>  // proc_pidpath
#include <unistd.h>
#include <fcntl.h>  // open
// sendfile
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#else
#include <unistd.h>
#include <fcntl.h>  // open
#include <sys/sendfile.h>
#endif

#include <array>
#include <algorithm>

namespace inviwo {

namespace detail {

// If path contains the location of a directory, it cannot contain a trailing backslash.
// If it does, stat will return -1 and errno will be set to ENOENT.
// https://msdn.microsoft.com/en-us/library/14h5k7ff.aspx
std::string removeTrailingSlash(const std::string& path) {
    // Remove trailing backslash or slash
    if (path.size() > 1 && (path.back() == '/' || path.back() == '\\')) {
        return path.substr(0, path.size() - 1);
    }
    return path;
}

}  // namespace detail

namespace filesystem {

FILE* fopen(const std::string& filename, const char* mode) {
#if defined(_WIN32)
    return _wfopen(util::toWstring(filename).c_str(), util::toWstring(mode).c_str());
#else
    return ::fopen(filename.c_str(), mode);
#endif
}

std::fstream fstream(const std::string& filename, std::ios_base::openmode mode) {
#if defined(_WIN32)
    return std::fstream(util::toWstring(filename), mode);
#else
    return std::fstream(filename, mode);
#endif
}

std::ifstream ifstream(const std::string& filename, std::ios_base::openmode mode) {
#if defined(_WIN32)
    return std::ifstream(util::toWstring(filename), mode);
#else
    return std::ifstream(filename, mode);
#endif
}

std::ofstream ofstream(const std::string& filename, std::ios_base::openmode mode) {
#if defined(_WIN32)
    return std::ofstream(util::toWstring(filename), mode);
#else
    return std::ofstream(filename, mode);
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
    std::array<char, FILENAME_MAX> workingDir;
#ifdef WIN32
    if (!GetCurrentDirectoryA(static_cast<DWORD>(workingDir.size()), workingDir.data()))
        throw Exception("Error querying current directory");
#else
    if (!getcwd(workingDir.data(), workingDir.size()))
        throw Exception("Error querying current directory");
#endif
    return cleanupPath(std::string(workingDir.data()));
}

std::string getExecutablePath() {
    // http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe
    std::string retVal;
#ifdef WIN32
    const DWORD maxBufSize = 1 << 20;  // corresponds to 1MiB
    std::vector<char> executablePath(FILENAME_MAX);

    auto size = GetModuleFileNameA(nullptr, executablePath.data(),
                                   static_cast<DWORD>(executablePath.size()));
    if (size == 0) throw Exception("Error retrieving executable path");
    while (size == executablePath.size()) {
        // buffer is too small, enlarge
        auto newSize = executablePath.size() * 2;
        if (newSize > maxBufSize) {
            throw Exception("Insufficient buffer size");
        }
        executablePath.resize(newSize);
        size = GetModuleFileNameA(nullptr, executablePath.data(),
                                  static_cast<DWORD>(executablePath.size()));
        if (size == 0) throw Exception("Error retrieving executable path");
    }
    retVal = std::string(executablePath.data());
#elif __APPLE__
    // http://stackoverflow.com/questions/799679/programatically-retrieving-the-absolute-path-of-an-os-x-command-line-app/1024933#1024933
    std::array<char, PROC_PIDPATHINFO_MAXSIZE> executablePath;
    auto pid = getpid();
    if (proc_pidpath(pid, executablePath.data(), executablePath.size()) <= 0) {
        // Error retrieving path
        throw Exception("Error retrieving executable path");
    }
    retVal = std::string(executablePath.data());
#else  // Linux
    std::array<char, FILENAME_MAX> executablePath;
    auto size = ::readlink("/proc/self/exe", executablePath.data(), executablePath.size() - 1);
    if (size != -1) {
        // readlink does not append a NUL character to the path
        executablePath[size] = '\0';
    } else {
        // Error retrieving path
        throw Exception("Error retrieving executable path");
    }
    retVal = std::string(executablePath.data());
#endif
    return retVal;
}

IVW_CORE_API std::string getInviwoUserSettingsPath() {
    std::stringstream ss;
#ifdef _WIN32
    PWSTR path;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path);
    if (SUCCEEDED(hr)) {
        char ch[1024];
        static const char DefChar = ' ';
        WideCharToMultiByte(CP_ACP, 0, path, -1, ch, 1024, &DefChar, nullptr);
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        auto tstConv = converter.to_bytes(path);
        CoTaskMemFree(path);
        ss << std::string(ch) << "/Inviwo";
    } else {
        throw Exception("SHGetKnownFolderPath failed to get settings folder",
                        IVW_CONTEXT_CUSTOM("filesystem"));
    }

#elif defined(__unix__)
    ss << std::getenv("HOME");
    ss << "/.inviwo";
#elif defined(__APPLE__)
    // Taken from:
    // http://stackoverflow.com/questions/5123361/finding-library-application-support-from-c?rq=1
    // A deprecated solution, but a solution...

    FSRef ref;
    OSType folderType = kApplicationSupportFolderType;
    int MAX_PATH = 512;
    char path[PATH_MAX];

#include <warn/push>
#include <warn/ignore/deprecated-declarations>
    FSFindFolder(kUserDomain, folderType, kCreateFolder, &ref);
    FSRefMakePath(&ref, (UInt8*)&path, MAX_PATH);
#include <warn/pop>
    ss << path << "/org.inviwo.network-editor";

#else
    LogWarnCustom("filesystem::getInviwoApplicationPath",
                  "Get User Setting Path is not implemented for current system");
#endif
    return ss.str();
}

bool fileExists(const std::string& filePath) {
// http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
#if defined(_WIN32)
    struct _stat buffer;
    return (_wstat(util::toWstring(filePath).c_str(), &buffer) == 0);
#else
    struct stat buffer;
    return (stat(filePath.c_str(), &buffer) == 0);
#endif
}

bool directoryExists(const std::string& path) {
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
    return (stat(detail::removeTrailingSlash(path).c_str(), &buffer) == 0 &&
            (buffer.st_mode & S_IFDIR));
#endif
}

std::time_t fileModificationTime(const std::string& filePath) {
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
    err = stat(detail::removeTrailingSlash(filePath).c_str(), &buffer);
#endif
    if (err != -1) {
        return buffer.st_mtime;
    } else {
        return 0;
    }
}

IVW_CORE_API bool copyFile(const std::string& src, const std::string& dst) {
#ifdef WIN32
    // Copy file and overwrite if it exists.
    // != 0 to get rid of bool comparison warning (C4800)
    return CopyFileA(src.c_str(), dst.c_str(), FALSE) != 0;
#else
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
    TinyDirInterface tinydir;
    switch (mode) {
        case ListMode::Files:
            tinydir.setListMode(TinyDirInterface::ListMode::FilesOnly);
            break;
        case ListMode::Directories:
            tinydir.setListMode(TinyDirInterface::ListMode::DirectoriesOnly);
            break;
        case ListMode::FilesAndDirectories:
            tinydir.setListMode(TinyDirInterface::ListMode::FilesAndDirectories);
            break;
    }
    tinydir.open(path);

    return tinydir.getContents();
}

std::vector<std::string> getDirectoryContentsRecursively(const std::string& path,
                                                         ListMode mode /*= ListMode::Files*/) {
    auto content = filesystem::getDirectoryContents(path, mode);
    auto directories = filesystem::getDirectoryContents(path, filesystem::ListMode::Directories);
    if (mode == ListMode::Directories || mode == ListMode::FilesAndDirectories) {
        // Remove . and ..
        util::erase_remove_if(content, [](const auto& dir) {
            return dir.compare(".") == 0 || dir.compare("..") == 0;
        });
    }

    for (auto& file : content) {
        file = path + "/" + file;
    }
    // Remove . and ..
    util::erase_remove_if(directories, [](const auto& dir) {
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

std::string getParentFolderWithChildren(const std::string& path,
                                        const std::vector<std::string>& childFolders) {
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
    return std::string();
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
            auto path = splitString(macPath, '/');
            path.pop_back();
            return joinString(path, "/");
        }
    }
#endif
    // locate Inviwo base path matching the subfolders data/workspaces and modules
    std::string basePath = inviwo::filesystem::getParentFolderWithChildren(
        inviwo::filesystem::getExecutablePath(), {"data/workspaces", "modules"});

    if (basePath.empty()) {
        // could not locate base path relative to executable, try CMake source path
        if (directoryExists(IVW_TRUNK + "/data/workspaces") &&
            directoryExists(IVW_TRUNK + "/modules")) {
            basePath = IVW_TRUNK;
        } else {
            throw Exception("Could not locate Inviwo base path");
        }
    }
    return basePath;
}

IVW_CORE_API std::string getPath(PathType pathType, const std::string& suffix,
                                 const bool createFolder) {
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

void createDirectoryRecursively(std::string path) {
    path = cleanupPath(path);
    std::vector<std::string> v = splitString(path, '/');

    std::string pathPart;
#ifdef _WIN32
    pathPart += v.front();
    v.erase(v.begin());
#endif

    while (!v.empty()) {
        pathPart += "/" + v.front();
        v.erase(v.begin());
#ifdef _WIN32
        _mkdir(pathPart.c_str());
#elif defined(__unix__)
        mkdir(pathPart.c_str(), 0755);
#elif defined(__APPLE__)
        mkdir(pathPart.c_str(), 0755);
#else
        LogWarnCustom("", "createDirectoryRecursively is not implemented for current system");
#endif
    }
}

// ---------- Helper function to retrieve inviwo settings folder -----------//
#ifdef _WIN32
static std::string helperSHGetKnownFolderPath(const KNOWNFOLDERID& id) {
    PWSTR path;
    HRESULT hr = SHGetKnownFolderPath(id, 0, nullptr, &path);
    std::string s = "";
    if (SUCCEEDED(hr)) {
        char ch[1024];
        static const char DefChar = ' ';
        WideCharToMultiByte(CP_ACP, 0, path, -1, ch, 1024, &DefChar, nullptr);
        s = std::string(ch);
    } else {
        LogErrorCustom("filesystem::getUserSettingsPath",
                       "SHGetKnownFolderPath failed to get settings folder");
    }

    CoTaskMemFree(path);
    return s;
}
#endif

std::string addBasePath(const std::string& url) {
    if (url.empty()) return findBasePath();
    return findBasePath() + "/" + url;
}

std::string getFileDirectory(const std::string& url) {
    std::string path = cleanupPath(url);
    size_t pos = path.rfind('/');
    if (pos == std::string::npos) return "";
    std::string fileDirectory = path.substr(0, pos);
    return fileDirectory;
}

std::string getFileNameWithExtension(const std::string& url) {
    std::string path = cleanupPath(url);
    size_t pos = path.rfind("/") + 1;
    // This relies on the fact that std::string::npos + 1 = 0
    std::string fileNameWithExtension = path.substr(pos, path.length());
    return fileNameWithExtension;
}

std::string getFileNameWithoutExtension(const std::string& url) {
    std::string fileNameWithExtension = getFileNameWithExtension(url);
    size_t pos = fileNameWithExtension.find_last_of(".");
    std::string fileNameWithoutExtension = fileNameWithExtension.substr(0, pos);
    return fileNameWithoutExtension;
}

std::string getFileExtension(const std::string& url) {
    std::string filename = getFileNameWithExtension(url);
    size_t pos = filename.rfind('.');

    if (pos == std::string::npos) return "";

    std::string fileExtension = filename.substr(pos + 1, url.length());
    return fileExtension;
}

std::string replaceFileExtension(const std::string& url, const std::string& newFileExtension) {
    size_t pos = url.rfind('.');
    std::string newUrl = url.substr(0, pos) + "." + newFileExtension;
    return newUrl;
}

std::string getRelativePath(const std::string& basePath, const std::string& absolutePath) {
    const std::string absPath(getFileDirectory(cleanupPath(absolutePath)));
    const std::string fileName(getFileNameWithExtension(cleanupPath(absolutePath)));

    // path as string tokens
    auto basePathTokens = splitString(basePath, '/');
    auto absolutePathTokens = splitString(absPath, '/');

    size_t sizediff = 0;
    if (basePathTokens.size() < absolutePathTokens.size()) {
        sizediff = absolutePathTokens.size() - basePathTokens.size();
    }
    auto start = std::mismatch(absolutePathTokens.begin(), absolutePathTokens.end() - sizediff,
                               basePathTokens.begin(), basePathTokens.end());

    // add one ".." for each unique folder in basePathTokens
    std::vector<std::string> relativePath(std::distance(start.second, basePathTokens.end()), "..");

    // add append the unique folders in absolutePathTokens
    std::copy(start.first, absolutePathTokens.end(), std::back_inserter(relativePath));

    if (!fileName.empty()) {
        relativePath.push_back(fileName);
    }

    return joinString(relativePath, "/");
}

std::string getCanonicalPath(const std::string& url) {
#ifdef WIN32
    const DWORD buffSize = 4096;  // MAX_PATH
    std::wstring urlWStr;
    urlWStr.assign(url.begin(), url.end());
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
        result.assign(resultWStr.begin(), resultWStr.end());
    }

    return result;
#else
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
    if (path.size() < 2) {
        return false;
    }

    // check for '[A-Z]:' in the begin of path
    char driveLetter = static_cast<char>(toupper(path[0]));
    return ((driveLetter >= 'A') && (driveLetter <= 'Z') && (path[1] == ':'));

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

std::string cleanupPath(const std::string& path) {
    if (path.empty()) {
        return path;
    }

    std::string result(path);
    // replace backslashes '\' with forward slashes '/'
    std::replace(result.begin(), result.end(), '\\', '/');

    // check for matching quotes at begin and end
    if ((result.size() > 1) && (result.front() == '\"') && (result.back() == '\"')) {
        result = result.substr(1, result.size() - 2);
    }
    return result;
}

}  // end namespace filesystem

}  // end namespace inviwo
