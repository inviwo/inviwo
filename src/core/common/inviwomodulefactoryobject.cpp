/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {

InviwoModuleFactoryObject::InviwoModuleFactoryObject(
    const std::string& name_, Version version_, const std::string& description_,
    Version inviwoCoreVersion_, std::vector<std::string> dependencies_,
    std::vector<Version> dependenciesVersion_, std::vector<std::string> aliases_,
    std::vector<LicenseInfo> licenses_, ProtectedModule protectedModule_)
    : name(name_)
    , version(version_)
    , description(description_)
    , inviwoCoreVersion(inviwoCoreVersion_)
    , dependencies([&]() {
        if (dependencies_.size() != dependenciesVersion_.size()) {
            throw Exception("Each module dependency must have a version");
        }
        std::vector<std::pair<std::string, Version>> deps;
        for (auto&& item : util::zip(dependencies_, dependenciesVersion_)) {
            deps.emplace_back(get<0>(item), Version(get<0>(item)));
        }
        return deps;
    }())
    , aliases(aliases_)
    , licenses(licenses_)
    , protectedModule(protectedModule_) {}

/**
 * \brief Sorts modules according to their dependencies.
 *
 * Recursive function that sorts the input vector according to their dependencies.
 * Modules depending on other modules will end up last in the list.
 *
 * @param start graph Objects to sort
 * @param end graph Objects to sort
 * @param lname Module dependency to sort
 * @param visited Modules already searched
 * @param tmpVisited Modules being searched
 * @param sorted Module names, sorted after dependencies.
 */
void recursiveTopologicalModuleFactoryObjectSort(
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>>::iterator start,
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>>::iterator end, const std::string& lname,
    std::unordered_set<std::string>& visited, std::unordered_set<std::string>& tmpVisited,
    std::vector<std::string>& sorted) {
    auto it = std::find_if(start, end, [&](const auto& a) { return toLower(a->name) == lname; });
    if (it == end) {  // This dependency has not been loaded
        return;
    }

    if (visited.find(lname) != visited.end()) return;  // Already visited;
    if (tmpVisited.find(lname) != tmpVisited.end()) {
        throw Exception("Dependency graph not a DAG", IVW_CONTEXT_CUSTOM("TopologicalModuleSort"));
    }

    tmpVisited.insert(lname);

    for (const auto& dependency : (*it)->dependencies) {
        auto ldep = toLower(dependency.first);
        recursiveTopologicalModuleFactoryObjectSort(start, end, ldep, visited, tmpVisited, sorted);
    }
    visited.insert(lname);
    sorted.insert(sorted.begin(), lname);

    return;
}

void topologicalModuleFactoryObjectSort(
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>>::iterator start,
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>>::iterator end) {
    // Topological sort to make sure that we load modules in correct order
    // https://en.wikipedia.org/wiki/Topological_sorting#Depth-first_search
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> tmpVisited;
    std::vector<std::string> sorted;
    for (auto it = start; it != end; ++it) {
        auto lname = toLower((*it)->name);
        if (visited.find(lname) == visited.end()) {
            recursiveTopologicalModuleFactoryObjectSort(start, end, lname, visited, tmpVisited,
                                                        sorted);
        }
    }
    // Sort modules according to dependency graph
    std::sort(start, end, [&](const auto& a, const auto& b) {
        return std::find(std::begin(sorted), std::end(sorted), toLower(a->name)) >
               std::find(std::begin(sorted), std::end(sorted), toLower(b->name));
    });
}

}  // namespace inviwo
