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
#include <inviwo/core/common/inviwoapplication.h>

// For directory exists
#include <sys/types.h>
#include <sys/stat.h>

// For working directory
#include <stdio.h>  // FILENAME_MAX

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
    return (stat(path.c_str(), &buffer) == 0 && (buffer.st_mode & S_IFDIR));
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

void createDirectoryRecursivly(std::string path) {
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
        LogWarnCustom("", "createDirectoryRecursivly is not implemented for current system");
#endif
    }
}

// ---------- Helper function to retrieve inviwo settings folder -----------//
#ifdef _WIN32
static std::string helperSHGetKnownFolderPath(const KNOWNFOLDERID& id) {
    PWSTR path;
    HRESULT hr = SHGetKnownFolderPath(id, 0, NULL, &path);
    std::string s = "";
    if (SUCCEEDED(hr)) {
        char ch[1024];
        static const char DefChar = ' ';
        WideCharToMultiByte(CP_ACP, 0, path, -1, ch, 1024, &DefChar, NULL);
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
    ss << "/Inviwo/";
#elif defined(__unix__)
    ss << std::getenv("HOME");
    ss << "/.inviwo/";
#elif defined(__APPLE__)
    // Taken from:
    // http://stackoverflow.com/questions/5123361/finding-library-application-support-from-c?rq=1
    // A depricated solution, but a solution...

    FSRef ref;
    OSType folderType = kApplicationSupportFolderType;
    int MAX_PATH = 512;
    char path[PATH_MAX];

    STARTCLANGIGNORE("-Wdeprecated-declarations")
    FSFindFolder(kUserDomain, folderType, kCreateFolder, &ref);
    FSRefMakePath(&ref, (UInt8*)&path, MAX_PATH);
    ENDCLANGIGNORE

    ss << path << "/org.inviwo.network-editor/";

#else
    LogWarnCustom("", "Get User Setting Path is not implemented for current system");
#endif
    return ss.str();
}

std::string addBasePath(const std::string& url) {
    return InviwoApplication::getPtr()->getBasePath() + "/" + url;
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
    if (basePath.empty()) basePath = InviwoApplication::getPtr()->getBasePath();

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
    for (size_t i = 0; i < basePathTokens.size(); i++)
        if (basePathTokens[i] != "") relativePath += "../";

    for (size_t i = 0; i < absolutePathTokens.size(); i++)
        if (absolutePathTokens[i] != "") relativePath += (absolutePathTokens[i] + "/");

    return relativePath + fileName;
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
            referencePath = InviwoApplication::getPtr()->getBasePath();
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