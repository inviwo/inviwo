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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/modulepath.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/vectoroperations.h>

#include <memory>
#include <vector>
#include <string>
#include <string_view>
#include <filesystem>

namespace inviwo {

class ModuleManager;
class InviwoModule;
class InviwoApplication;

namespace util {

IVW_CORE_API ModuleManager& getModuleManager();
IVW_CORE_API ModuleManager& getModuleManager(InviwoApplication* app);

IVW_CORE_API InviwoModule* getModuleByIdentifier(std::string_view identifier);
IVW_CORE_API InviwoModule* getModuleByIdentifier(InviwoApplication* app,
                                                 std::string_view identifier);

IVW_CORE_API size_t getNumberOfModules();
IVW_CORE_API size_t getNumberOfModules(InviwoApplication* app);

IVW_CORE_API InviwoModule* getModuleByIndex(size_t index);
IVW_CORE_API InviwoModule* getModuleByIndex(InviwoApplication* app, size_t index);

IVW_CORE_API std::filesystem::path getModulePath(InviwoModule* module);
IVW_CORE_API std::filesystem::path getModulePath(InviwoModule* module, ModulePath pathType);

/**
 * \brief return the path for a specific type located within the requested module
 *
 * @param identifier   name of the module
 * @param pathType     type of the requested path
 * @return subdirectory of the module matching the type
 */
IVW_CORE_API std::filesystem::path getModulePath(std::string_view identifier, ModulePath pathType);
IVW_CORE_API std::filesystem::path getModulePath(InviwoApplication* app,
                                                 std::string_view identifier, ModulePath pathType);
IVW_CORE_API std::filesystem::path getModulePath(std::string_view identifier);
IVW_CORE_API std::filesystem::path getModulePath(InviwoApplication* app,
                                                 std::string_view identifier);

template <class T>
T* getModuleByType(InviwoApplication* app) {
    const auto size = getNumberOfModules(app);
    for (size_t i = 0; i < size; ++i) {
        if (auto m = dynamic_cast<T*>(getModuleByIndex(app, i))) {
            return m;
        }
    }
    return nullptr;
}

template <class T>
T* getModuleByType() {
    const auto size = getNumberOfModules();
    for (size_t i = 0; i < size; ++i) {
        if (auto m = dynamic_cast<T*>(getModuleByIndex(i))) {
            return m;
        }
    }
    return nullptr;
}

/**
 * \brief return the path for a specific type located within the requested module of type T
 *
 * @param pathType     type of the requested path
 * @return subdirectory of the module matching the type
 */
template <typename T>
std::filesystem::path getModulePath(ModulePath pathType) {
    std::filesystem::path path;
    if (auto m = getModuleByType<T>()) {
        path = m->getPath(pathType);
        if (path.empty() || path == m->getPath()) {
            throw Exception("Could not locate module path for specified path type",
                            IVW_CONTEXT_CUSTOM("module::getModulePath"));
        }
    } else {
        throw Exception("Could not locate module", IVW_CONTEXT_CUSTOM("module::getModulePath"));
    }
    return path;
}

template <typename T>
std::filesystem::path getModulePath(InviwoApplication* app, ModulePath pathType) {
    std::filesystem::path path;
    if (auto m = getModuleByType<T>(app)) {
        path = m->getPath(pathType);
        if (path.empty() || path == m->getPath()) {
            throw Exception("Could not locate module path for specified path type",
                            IVW_CONTEXT_CUSTOM("module::getModulePath"));
        }
    } else {
        throw Exception("Could not locate module", IVW_CONTEXT_CUSTOM("module::getModulePath"));
    }
    return path;
}

}  // namespace util

}  // namespace inviwo
