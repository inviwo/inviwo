/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2016 Inviwo Foundation
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

namespace inviwo {

InviwoModuleFactoryObject::InviwoModuleFactoryObject(const std::string& name_,
                                                     const std::string& version_,
                                                     const std::string& description_,
                                                     const std::string& inviwoCoreVersion_,
                                                     std::vector<std::string> dependencies_,
                                                     std::vector<std::string> dependenciesVersion_)
    : name(name_)
    , version(version_)
    , description(description_)
    , inviwoCoreVersion(inviwoCoreVersion_)
    , dependencies(dependencies_)
    , dependenciesVersion(dependenciesVersion_) {
    if (dependencies.size() != dependenciesVersion.size()) {
        throw Exception("Each module dependency must have a version");
    }
}

/**
 * \brief Sorts modules according to their dependencies.
 *
 * Recursive function that sorts the input vector according to their dependencies.
 * Modules depending on other modules will end up last in the list.
 *
 * @param std::vector<std::unique_ptr<InviwoModuleFactoryObject>> & graph Objects to sort
 * @param std::unordered_set<std::string> & explored Modules already searched
 * @param std::string moduleDependency Module dependency to sort
 * @param std::vector<std::string> & sorted Module names, sorted after dependencies.
 * @param size_t & t Number of elements in graph.
 */
void recursiveTopologicalModuleFactoryObjectSort(
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>>::iterator start,
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>>::iterator end,
    std::unordered_set<std::string>& explored, std::string i, std::vector<std::string>& sorted,
    size_t& t) {
    auto it = std::find_if(start, end, [&](const std::unique_ptr<InviwoModuleFactoryObject>& a) {
        // Lower case comparison
        return toLower(a->name) == toLower(i);
    });
    if (it == end) {
        // This dependency has not been loaded
        return;
    }
    explored.insert(toLower((*it)->name));

    for (const auto& dependency : (*it)->dependencies) {
        auto lowerCaseDependency = toLower(dependency);
        if (explored.find(lowerCaseDependency) == explored.end()) {
            recursiveTopologicalModuleFactoryObjectSort(start, end, explored, lowerCaseDependency,
                                                        sorted, t);
        }
    }

    --t;
    sorted[t] = i;

    return;
}

void topologicalModuleFactoryObjectSort(
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>>::iterator start,
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>>::iterator end) {
    // Topological sort to make sure that we load modules in correct order
    // https://en.wikipedia.org/wiki/Topological_sorting#Depth-first_search
    std::unordered_set<std::string> explored;
    size_t t = std::distance(start, end);
    std::vector<std::string> sorted(t);
    for (auto moduleIt = start; moduleIt != end; ++moduleIt) {
        const auto& module = *moduleIt;
        auto lowerCaseName = toLower(module->name);
        if (explored.find(lowerCaseName) == explored.end()) {
            recursiveTopologicalModuleFactoryObjectSort(start, end, explored, lowerCaseName, sorted,
                                                        t);
        }
    }
    // Sort modules according to dependency graph
    std::sort(start, end, [&](const std::unique_ptr<InviwoModuleFactoryObject>& a,
                              const std::unique_ptr<InviwoModuleFactoryObject>& b) {
        return std::find(std::begin(sorted), std::end(sorted), toLower(a->name)) >
               std::find(std::begin(sorted), std::end(sorted), toLower(b->name));
    });
}

}  // namespace
