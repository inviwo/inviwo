/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <memory>
#include <memory_resource>

namespace inviwo::util {

struct IVW_CORE_API PMRDeleter {
    explicit PMRDeleter(std::pmr::polymorphic_allocator<std::byte> alloc)
        : resource{alloc.resource()} {}
    PMRDeleter(const PMRDeleter&) = default;
    PMRDeleter(PMRDeleter&&) = default;
    PMRDeleter& operator=(const PMRDeleter&) = default;
    PMRDeleter& operator=(PMRDeleter&&) = default;
    ~PMRDeleter() = default;

    template <typename T>
    void operator()(T* item) {
        std::pmr::polymorphic_allocator<std::byte>{resource}.delete_object(item);
    }
    std::pmr::memory_resource* resource;
};

template <typename T>
using PMRUnique = std::unique_ptr<T, PMRDeleter>;

template <class T, class... Args>
PMRUnique<T> pmr_make_unique(std::pmr::polymorphic_allocator<std::byte> alloc, Args&&... args) {
    return PMRUnique<T>(alloc.new_object<T>(std::forward<Args>(args)...), PMRDeleter{alloc});
}

}  // namespace inviwo::util
