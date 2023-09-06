/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2023 Inviwo Foundation
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

#include <inviwo/core/util/moduleutils.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>

namespace inviwo {

namespace util {

ModuleManager& getModuleManager(InviwoApplication* app) { return app->getModuleManager(); }

ModuleManager& getModuleManager() { return getModuleManager(InviwoApplication::getPtr()); }

size_t getNumberOfModules() { return getNumberOfModules(InviwoApplication::getPtr()); }
size_t getNumberOfModules(InviwoApplication* app) { return getModuleManager(app).size(); }

InviwoModule* getModuleByIndex(size_t index) {
    return getModuleByIndex(InviwoApplication::getPtr(), index);
}
InviwoModule* getModuleByIndex(InviwoApplication* app, size_t index) {
    return getModuleManager(app).getModuleByIndex(index);
}

InviwoModule* getModuleByIdentifier(InviwoApplication* app, std::string_view identifier) {
    return getModuleManager(app).getModuleByIdentifier(identifier);
}
InviwoModule* getModuleByIdentifier(std::string_view identifier) {
    return getModuleByIdentifier(InviwoApplication::getPtr(), identifier);
}

std::filesystem::path getModulePath(InviwoModule* module) { return module->getPath(); }

std::filesystem::path getModulePath(InviwoModule* module, ModulePath pathType) {
    return module->getPath(pathType);
}

std::filesystem::path getModulePath(InviwoApplication* app, std::string_view identifier) {
    if (auto m = getModuleByIdentifier(app, identifier)) {
        return m->getPath();
    } else {
        throw Exception(IVW_CONTEXT_CUSTOM("module::getModulePath"),
                        "Could not locate module \"{}\"", identifier);
    }
}

std::filesystem::path getModulePath(std::string_view identifier) {
    return getModulePath(InviwoApplication::getPtr(), identifier);
}
std::filesystem::path getModulePath(InviwoApplication* app, std::string_view identifier,
                                    ModulePath pathType) {
    std::filesystem::path path;
    if (auto m = getModuleByIdentifier(app, identifier)) {
        path = m->getPath(pathType);
        if (path.empty() || path == m->getPath()) {
            // if the result of getPath(pathType) is identical with getPath(),
            // i.e. the module path, the specific path does not exist.
            throw Exception(IVW_CONTEXT_CUSTOM("module::getModulePath"),
                            "Could not locate module path for specified path type (module \"{}\")",
                            identifier);
        }
    } else {
        throw Exception(IVW_CONTEXT_CUSTOM("module::getModulePath"),
                        "Could not locate module \"{}\"", identifier);
    }
    return path;
}

std::filesystem::path getModulePath(std::string_view identifier, ModulePath pathType) {
    return getModulePath(InviwoApplication::getPtr(), identifier, pathType);
}

}  // namespace util

}  // namespace inviwo
