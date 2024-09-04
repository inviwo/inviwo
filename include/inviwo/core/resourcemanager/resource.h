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
#include <inviwo/core/util/formats.h>
#include <glm/vec4.hpp>
#include <glm/gtx/component_wise.hpp>
#include <string_view>
#include <cstdint>
#include <optional>

namespace inviwo {

struct IVW_CORE_API ResourceMeta {
    std::string source;
};

struct IVW_CORE_API Resource {
    glm::size4_t dims{0, 0, 0, 0};
    DataFormatId format{DataFormatId::NotSpecialized};
    std::string_view desc{};
    std::optional<ResourceMeta> meta{std::nullopt};

    size_t sizeInBytes() const {
        return glm::compMul(glm::max(dims, glm::size4_t{1, 1, 1, 1})) *
               DataFormatBase::get(format)->getSizeInBytes();
    }
};

namespace resource {
struct IVW_CORE_API RAM {
    std::uintptr_t key;
    auto operator<=>(const RAM&) const = default;
    static constexpr std::string_view name = "RAM";
};
struct IVW_CORE_API GL {
    unsigned int key;
    auto operator<=>(const GL&) const = default;
    static constexpr std::string_view name = "GL";
};
struct IVW_CORE_API PY {
    size_t key;
    auto operator<=>(const PY&) const = default;
    static constexpr std::string_view name = "PY";
};

IVW_CORE_API RAM toRAM(const void* ptr);

template <typename T>
RAM toRAM(const std::unique_ptr<T>& data) {
    return toRAM(static_cast<const void*>(data.get()));
}

IVW_CORE_API void add(const RAM& key, Resource resource);
IVW_CORE_API std::optional<Resource> remove(const RAM& key);
IVW_CORE_API void meta(const RAM& key, const ResourceMeta& meta);

IVW_CORE_API void add(const GL& key, Resource resource);
IVW_CORE_API std::optional<Resource> remove(const GL& key);
IVW_CORE_API void meta(const GL& key, const ResourceMeta& meta);

IVW_CORE_API void add(const PY& key, Resource resource);
IVW_CORE_API std::optional<Resource> remove(const PY& key);
IVW_CORE_API void meta(const PY& key, const ResourceMeta& meta);

constexpr auto getMeta(const std::optional<Resource>& r) -> std::optional<ResourceMeta> {
    if (r) return r->meta;
    return std::nullopt;
};
constexpr auto getMeta(std::optional<Resource>&& r) -> std::optional<ResourceMeta> {
    if (r) return std::move(r->meta);
    return std::nullopt;
};

}  // namespace resource

}  // namespace inviwo

template <>
struct std::hash<inviwo::resource::RAM> {
    size_t operator()(const inviwo::resource::RAM& item) const {
        return std::hash<decltype(item.key)>{}(item.key);
    }
};
template <>
struct std::hash<inviwo::resource::GL> {
    size_t operator()(const inviwo::resource::GL& item) const {
        return std::hash<decltype(item.key)>{}(item.key);
    }
};
template <>
struct std::hash<inviwo::resource::PY> {
    size_t operator()(const inviwo::resource::PY& item) const {
        return std::hash<decltype(item.key)>{}(item.key);
    }
};
