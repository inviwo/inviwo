/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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
// Used for registering modules (see globalmacros.cmake, ivw_private_generate_module_registration_file)

#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/util/sharedlibrary.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/logcentral.h>

#if defined(__unix__)
#include <elf.h> // To retrieve rpath
#include <link.h>
#endif

namespace inviwo {

/**
 * \brief Returns paths to search for module libraries.
 * All platforms: executable directory and application modules directory
 * (AppData/Inviwo/modules on windows).
 * Platform dependent search directories:
 * OSX: DYLD_LIBRARY_PATH
 * UNIX: LD_LIBRARY_PATH/LD_RUN_PATH, RPATH and "executable directory
 * /../../lib"
 * @return List of paths to directories
 */
std::vector<std::string> getModuleList() {
    auto paths = std::vector<std::string>{
        inviwo::filesystem::getFileDirectory(inviwo::filesystem::getExecutablePath()),
        inviwo::filesystem::getPath(inviwo::PathType::Modules)};

    // http://unix.stackexchange.com/questions/22926/where-do-executables-look-for-shared-objects-at-runtime
#if defined(__APPLE__)
    // Xcode/OSX store library output path in DYLD_LIBRARY_PATH
    if (char *envPaths = std::getenv("DYLD_LIBRARY_PATH")) {
        auto libPaths = splitString(envPaths, ':');
        paths.insert(std::end(paths), std::begin(libPaths), std::end(libPaths));
    }
#elif defined(__unix__)
    paths.push_back(inviwo::filesystem::getFileDirectory(
        inviwo::filesystem::getExecutablePath()) +
        "/../lib");
    // Unix uses LD_LIBRARY_PATH or LD_RUN_PATH
    if (char *envPaths = std::getenv("LD_LIBRARY_PATH")) {
        auto libPaths = splitString(envPaths, ':');
        paths.insert(std::end(paths), std::begin(libPaths), std::end(libPaths));
    }
    if (char *envPaths = std::getenv("LD_RUN_PATH")) {
        auto libPaths = splitString(envPaths, ':');
        paths.insert(std::end(paths), std::begin(libPaths), std::end(libPaths));
    }
    // Additional paths can be specified in
    // ELF header: RUN_PATH or RPATH
    const ElfW(Dyn) *rPath = nullptr;
    const ElfW(Dyn) *runPath = nullptr;
    const char *offset = nullptr;
    for (const ElfW(Dyn) *dyn = _DYNAMIC; dyn->d_tag != DT_NULL; ++dyn) {
        if (dyn->d_tag == DT_RUNPATH) {
            runPath = dyn;
        } else if (dyn->d_tag == DT_RPATH) {
            rPath = dyn;
        } else if (dyn->d_tag == DT_STRTAB) {
            offset = (const char *)dyn->d_un.d_val;
        }
    }
    if (offset) {
        // Prioritize DT_RUNPATH, DT_RPATH is deprecated
        if (runPath) {
            auto rPaths = splitString(offset + runPath->d_un.d_val, ':');
            auto execPath = inviwo::filesystem::getExecutablePath();
            for (auto &path : rPaths) {
                replaceInString(path, "$ORIGIN", execPath);
            }
            paths.insert(std::end(paths), std::begin(rPaths), std::end(rPaths));
        } else if (rPath) {
            auto rPaths = splitString(offset + rPath->d_un.d_val, ':');
            auto execPath = inviwo::filesystem::getExecutablePath();
            for (auto &path : rPaths) {
                replaceInString(path, "$ORIGIN", execPath);
            }
            paths.insert(std::end(paths), std::begin(rPaths), std::end(rPaths));
        }
    }
#endif
    return paths;
}

}  //namespace
