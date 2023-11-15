/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2023 Inviwo Foundation
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

#include <inviwo/sys/inviwosysdefine.h>
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/util/foreacharg.h>
#include <inviwo/core/common/modulecontainer.h>
#include <inviwo/core/common/modulemanager.h>

#include <vector>
#include <span>
#include <filesystem>
#include <concepts>
#include <ranges>

namespace inviwo {

namespace util {

IVW_SYS_API std::vector<ModuleContainer> getModuleContainersImpl(
    ModuleManager& moduleManager, std::span<const std::filesystem::path> searchPaths);

template <typename... Args>
std::vector<inviwo::ModuleContainer> getModuleContainers(ModuleManager& moduleManager,
                                                         Args&&... searchPaths) {
    std::vector<std::filesystem::path> paths;

    util::for_each_argument(
        [&](auto&& arg) {
            if constexpr (std::constructible_from<std::filesystem::path, decltype(arg)>) {
                paths.emplace_back(arg);
            } else {
                std::ranges::for_each(arg, [&](auto& item) { paths.emplace_back(item); });
            }
        },
        searchPaths...);

    return getModuleContainersImpl(moduleManager, paths);
}

template <typename Filter, typename... Args>
void registerModulesFiltered(ModuleManager& moduleManager, Filter&& filter, Args&&... searchPaths) {
    auto inviwoModules = getModuleContainers(moduleManager, searchPaths...);
    std::erase_if(inviwoModules, filter);
    moduleManager.registerModules(std::move(inviwoModules));
}

template <typename... Args>
void registerModules(ModuleManager& moduleManager, Args&&... searchPaths) {
    auto inviwoModules = getModuleContainers(moduleManager, searchPaths...);
    moduleManager.registerModules(std::move(inviwoModules));
}

}  // namespace util

}  // namespace inviwo
