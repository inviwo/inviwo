/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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
#include <inviwo/core/resourcemanager/resource.h>
#include <inviwo/core/resourcemanager/resourcemanagerobserver.h>
#include <inviwo/core/util/foreacharg.h>
#include <inviwo/core/util/stdextensions.h>

#include <unordered_map>
#include <tuple>
#include <variant>

namespace inviwo {

class IVW_CORE_API ResourceManager : public ResourceManagerObservable {
public:
    using Keys = std::tuple<resource::RAM, resource::GL, resource::PY>;
    using Data = std::tuple<std::vector<std::pair<resource::RAM, Resource>>,
                            std::vector<std::pair<resource::GL, Resource>>,
                            std::vector<std::pair<resource::PY, Resource>>>;
    using Var = std::variant<std::monostate, const std::vector<std::pair<resource::RAM, Resource>>*,
                             const std::vector<std::pair<resource::GL, Resource>>*,
                             const std::vector<std::pair<resource::PY, Resource>>*>;
    static constexpr std::array<std::string_view, 3> names = {"RAM", "GL", "PY"};

    template <typename Key>
    void add(const Key& key, Resource resource) {
        constexpr auto gi = groupIndex<Key>();
        auto& group = get<Key>();

        auto it = std::ranges::find(group, key, &std::pair<Key, Resource>::first);
        if (it == group.end()) {
            notifyWillAddResource(gi, group.size(), resource);
            group.emplace_back(key, std::move(resource));
            notifyDidAddResource(gi, group.size() - 1, group.back().second);
        } else {
            notifyWillUpdateResource(gi, std::distance(group.begin(), it), it->second);
            it->second = std::move(resource);
            notifyDidUpdateResource(gi, std::distance(group.begin(), it), it->second);
        }
    }

    template <typename Key>
    void meta(const Key& key, const ResourceMeta& meta) {
        constexpr auto gi = groupIndex<Key>();
        auto& group = get<Key>();

        auto it = std::ranges::find(group, key, &std::pair<Key, Resource>::first);
        if (it != group.end()) {
            notifyWillUpdateResource(gi, std::distance(group.begin(), it), it->second);
            it->second.meta = meta;
            notifyDidUpdateResource(gi, std::distance(group.begin(), it), it->second);
        }
    }

    template <typename Key>
    std::optional<Resource> remove(const Key& key) {
        constexpr auto gi = groupIndex<Key>();
        auto& group = get<Key>();
        auto it = std::ranges::find(group, key, &std::pair<Key, Resource>::first);
        if (it != group.end()) {
            notifyWillRemoveResource(gi, std::distance(group.begin(), it), it->second);
            Resource resource{std::move(it->second)};
            it = group.erase(it);
            notifyDidRemoveResource(gi, std::distance(group.begin(), it), resource);
            return resource;
        } else {
            return std::nullopt;
        }
    }

    size_t totalSize() const {
        return std::apply([](auto&&... args) { return (args.size() + ...); }, data_);
    }

    size_t size(size_t groupIndex) const {
        return std::visit(util::overloaded{[](std::monostate) -> size_t { return 0; },
                                           [](const auto& list) -> size_t { return list->size(); }},
                          getGroup(groupIndex));
    }

    size_t totalByteSize(size_t groupIndex) const {
        static constexpr auto byteSize = [](const auto& elem) { return elem.second.sizeInBytes(); };
        static constexpr auto sum = [](const auto& vec) {
            return std::transform_reduce(vec->begin(), vec->end(), size_t{0}, std::plus<>(),
                                         byteSize);
        };
        return std::visit(util::overloaded{[](std::monostate) -> size_t { return 0; },
                                           [](const auto& list) -> size_t { return sum(list); }},
                          getGroup(groupIndex));
    }

    const Resource* get(size_t groupIndex, size_t index) const {
        return std::visit(
            util::overloaded{[](std::monostate) -> const Resource* { return nullptr; },
                             [index](const auto* list) -> const Resource* {
                                 if (index < list->size()) {
                                     return &(*list)[index].second;
                                 } else {
                                     return nullptr;
                                 }
                             }},
            getGroup(groupIndex));
    }

    void clear() {
        util::for_each_in_tuple(
            [this](auto& vec) {
                while (!vec.empty()) {
                    remove(vec.back().first);
                }
            },
            data_);         
    }

private:
    auto getGroup(size_t groupIndex) const -> Var {
        Var v{};
        util::for_each_in_tuple(
            [&, i = size_t{0}](auto& vec) mutable {
                if (i == groupIndex) {
                    v = &vec;
                }
                ++i;
            },
            data_);
        return v;
    }

    template <typename Key>
    std::vector<std::pair<Key, Resource>>& get() {
        return std::get<std::vector<std::pair<Key, Resource>>>(data_);
    }
    template <typename Key>
    const std::vector<std::pair<Key, Resource>>& get() const {
        return std::get<std::vector<std::pair<Key, Resource>>>(data_);
    }

    template <typename Key>
    static constexpr size_t groupIndex() {
        return util::index_of<Key, Keys>();
    }

    Data data_;
};

}  // namespace inviwo
