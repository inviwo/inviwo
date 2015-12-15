/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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
#include <inviwo/core/util/tinydirinterface.h>

// For directory exists
#include <sys/types.h>
#include <sys/stat.h>

// For working directory
#include <stdio.h>  // FILENAME_MAX

#include <cctype> // isdigit()

#ifdef WIN32
#include <windows.h>
#include <tchar.h>
#include <direct.h>
#include <Shlobj.h>
#elif defined(__APPLE__)
#include <CoreServices/CoreServices.h>
#else
#include <unistd.h>
#endif

namespace inviwo {

namespace filesystem {

std::string getWorkingDirectory() {
    char workingDir[FILENAME_MAX];
#ifdef WIN32

    if (!GetCurrentDirectoryA(sizeof(workingDir), workingDir)) return "";

#else

    if (!getcwd(workingDir, sizeof(workingDir))) return "";

#endif
    return std::string(workingDir);
}

bool fileExists(const std::string& filePath) {
    // http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
    struct stat buffer;
    return (stat(filePath.c_str(), &buffer) == 0);
}

bool directoryExists(const std::string& path) {
    struct stat buffer;
    // If path contains the location of a directory, it cannot contain a trailing backslash. 
    // If it does, -1 will be returned and errno will be set to ENOENT.
    // https://msdn.microsoft.com/en-us/library/14h5k7ff.aspx
    // We therefore check if path ends with a backslash
    if (path.size() > 1 && (path.back() == '/' || path.back() == '\\')) {
        // Remove trailing backslash
        std::string pathWithoutSlash = path.substr(0, path.size() - 1);
        return (stat(pathWithoutSlash.c_str(), &buffer) == 0 && (buffer.st_mode & S_IFDIR));
    } else {
        // No need to modify path
        return (stat(path.c_str(), &buffer) == 0 && (buffer.st_mode & S_IFDIR));
    }
}

std::vector<std::string> getDirectoryContents(const std::string& path) {
    if (path.empty()) {
        return{};
    }
    TinyDirInterface tinydir;
    tinydir.open(path);
    return tinydir.getContents();
}

bool wildcardStringMatch(const std::string &pattern, const std::string &str) {
    const char* patternPtr = pattern.c_str();
    const char* strPtr = str.c_str();

    const char* patternPtrSave = nullptr;
    const char* strPtrSave = nullptr;

    while (*strPtr != '\0') {
        if (*patternPtr == '*') {
            // wildcard detected
            ++patternPtr; // remove wildcard from pattern
            if (*patternPtr == '\0') {
                // wildcard at the end of the pattern matches remaining string
                return true;
            }
            patternPtrSave = patternPtr; // save position after wildcard for roll-back
            strPtrSave = strPtr + 1;
        }
        else if ((*patternPtr == '?') || (*patternPtr == *strPtr)) {
            // single character match or single character wildcard
            ++patternPtr;
            ++strPtr;
        }
        else if (!strPtrSave) {
            // early exit if the next string character is at the end
            return false;
        }
        else {
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

bool wildcardStringMatchDigits(const std::string &pattern, const std::string &str,
    int &index, bool matchLess, bool matchMore) {

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
                strPtrSave = nullptr; // reset wildcard mode for regular pattern matching
            }
        }
        else if (*patternPtr == '*') {
            // wildcard detected
            ++patternPtr; // remove wildcard from pattern
            if (*patternPtr == '\0') {
                // wildcard at the end of the pattern matches remaining string
                index = result;
                return true;
            }
            patternPtrSave = patternPtr; // save position after wildcard for roll-back
            strPtrSave = strPtr + 1;
        }
        else if ((*patternPtr == '?') || (*patternPtr == *strPtr)) {
            // single character match or single character wildcard
            ++patternPtr;
            ++strPtr;
        }
        else if (!strPtrSave) {
            // early exit if the next string character is at the end
            return false;
        }
        else {
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

std::string getParentFolderPath(const std::string& basePath, const std::string& parentFolder) {
    size_t pos = basePath.length();
    std::string fileDirectory = basePath;

    do {
        fileDirectory = fileDirectory.substr(0, pos);
        std::string moduleDirectory = fileDirectory + "/" + parentFolder;
        bool exists = directoryExists(moduleDirectory);

        if (exists) return fileDirectory;
    } while ((pos = fileDirectory.find_last_of("\\/")) != std::string::npos);

    return basePath;
}

std::string findBasePath() {
    // Search for directory containing data folder to find application basepath.
    // Working directory will be used if data folder is not found in parent directories.
    
    #if defined(__APPLE__) 
    // We might be in a bundle
    // the executable will be in
    //    Inviwo.app/Contents/MacOS/Inviwo
    // and the our base should be
    //    Inviwo.app/Contents/Resources
    
    CFURLRef appUrlRef = CFBundleCopyResourceURL(CFBundleGetMainBundle(), CFSTR("data"), NULL, NULL);
    if(appUrlRef) {
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
    
    static const std::string lookForWorkspaces = "/data/workspaces";
    std::string basePath =
        inviwo::filesystem::getParentFolderPath(inviwo::filesystem::getWorkingDirectory(), lookForWorkspaces);

    // If we did not find "data" in basepath, and if it doesn't contain modules, check CMake source path.
    static const std::string lookForModules = "/modules";
    if ((!directoryExists(basePath + lookForWorkspaces) || !directoryExists(basePath + lookForModules))
        && (directoryExists(IVW_TRUNK + lookForWorkspaces) && directoryExists(IVW_TRUNK + lookForModules))){
            basePath = IVW_TRUNK;
    }
    return basePath;
}

IVW_CORE_API std::string getPath(PathType pathType, const std::string& suffix, const bool& createFolder) {
    std::string result = findBasePath();

    switch (pathType) {
        case PathType::Data:
            result += "/data";
            break;

        case PathType::Volumes:
            result += "/data/volumes";
            break;

        case PathType::Modules:
            result += "/modules";
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
    replaceInString(path, "\\", "/");
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

std::string getInviwoUserSettingsPath() {
    std::stringstream ss;
#ifdef _WIN32
    ss << helperSHGetKnownFolderPath(FOLDERID_RoamingAppData);
    ss << "/Inviwo";
#elif defined(__unix__)
    ss << std::getenv("HOME");
    ss << "/.inviwo";
#elif defined(__APPLE__)
    // Taken from:
    // http://stackoverflow.com/questions/5123361/finding-library-application-support-from-c?rq=1
    // A depricated solution, but a solution...

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
    LogWarnCustom("", "Get User Setting Path is not implemented for current system");
#endif
    return ss.str();
}

std::string addBasePath(const std::string& url) {
    if (url.empty()) return findBasePath();
    return findBasePath() + "/" + url;
}

std::string getFileDirectory(const std::string& url) {
    size_t pos = url.find_last_of("\\/") + 1;
    std::string fileDirectory = url.substr(0, pos);
    return fileDirectory;
}

std::string getFileNameWithExtension(const std::string& url) {
    size_t pos = url.find_last_of("\\/") + 1;
    std::string fileNameWithExtension = url.substr(pos, url.length());
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
    size_t pos = filename.rfind(".");

    if (pos == std::string::npos) return "";

    std::string fileExtension = filename.substr(pos + 1, url.length());
    return fileExtension;
}

std::string replaceFileExtension(const std::string& url, const std::string& newFileExtension) {
    size_t pos = url.find_last_of(".") + 1;
    std::string newUrl = url.substr(0, pos) + newFileExtension;
    return newUrl;
}

std::string getRelativePath(const std::string& bPath, const std::string& absolutePath) {
    // FIXME: is the case that the bath path and the absolute path are lying on different drives
    // considered?
    // FIXME: different drives don't matter, since the first path token will be different (split
    // only for '/' and '\\')
    // FIXME: however, we have to make sure, both paths are absolute!
    std::string basePath(getFileDirectory(bPath));
    std::string absPath(getFileDirectory(absolutePath));
    std::string fileName(getFileNameWithExtension(absolutePath));
    std::string relativePath("");

    // if given base path is empty use system base path
    if (basePath.empty()) basePath = findBasePath();

    // path as string tokens
    std::vector<std::string> basePathTokens;
    std::vector<std::string> absolutePathTokens;
    size_t pos = 0, pos1 = std::string::npos;

    while (pos != std::string::npos) {
        pos1 = basePath.find_first_of("\\/", pos);

        if (pos1 != pos) basePathTokens.push_back(basePath.substr(pos, pos1 - pos));

        pos = basePath.find_first_not_of("\\/", pos1);
    }

    pos = 0, pos1 = std::string::npos;

    while (pos != std::string::npos) {
        pos1 = absPath.find_first_of("\\/", pos);

        if (pos1 != pos) absolutePathTokens.push_back(absPath.substr(pos, pos1 - pos));

        pos = absPath.find_first_not_of("\\/", pos1);
    }

    // discard matching tokens
    for (size_t i = 0; (i < basePathTokens.size() && i < absolutePathTokens.size()); i++) {
        if (basePathTokens[i] == absolutePathTokens[i])
            basePathTokens[i] = absolutePathTokens[i] = "";
        else
            break;
    }

    // handle non-matching tokens
    for (auto& basePathToken : basePathTokens)
        if (basePathToken != "") relativePath += "../";

    for (auto& absolutePathToken : absolutePathTokens)
        if (absolutePathToken != "") relativePath += (absolutePathToken + "/");

    if (fileName.empty() && !relativePath.empty()) {
        // remove trailing '/' from path
        std::size_t pathLen{ relativePath.size() };
        if (relativePath[pathLen - 1] == '/') {
            return relativePath.substr(0, pathLen);
        }
    }

    return relativePath + fileName;
}

std::string getCanonicalPath(const std::string& url) {
#ifdef WIN32
    const DWORD buffSize = 4096; // MAX_PATH
    std::wstring urlWStr;
    urlWStr.assign(url.begin(), url.end());
    std::string result{ url };

    TCHAR buffer[buffSize + 1];

    DWORD retVal = GetFullPathName(urlWStr.c_str(), buffSize, buffer, nullptr);
    if (retVal == 0) {
        // something went wrong, call GetLastError() to get the error code
        return result;
    }
    else if (retVal > buffSize) {
        // canonical path would be longer than buffer
        return result;
    }
    else {
        std::wstring resultWStr{ buffer };
        result.assign(resultWStr.begin(), resultWStr.end());
    }

    return result;
#else 
    char buffer[PATH_MAX + 1];
    char *retVal = realpath(url.c_str(), buffer);
    if (retVal == nullptr) {
        // something went wrong, check errno for error
        return url;
    }
    else {
        return std::string{ retVal };
    }
#endif
}

bool isAbsolutePath(const std::string& path) {
#ifdef WIN32
    if (path.size() < 2) {
        return false;
    }

    // check for '[A-Z]:' in the begin of path
    char driveLetter = toupper(path[0]);
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

}  // end namespace filesystem

}  // end namespace inviwo