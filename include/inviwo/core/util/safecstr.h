/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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

#include <string_view>
#include <array>
#include <memory>
#include <cstring>

namespace inviwo {

/**
 * @brief Safe and efficient conversion of a string_view to a null terminated c-string
 * Uses a internal char buffer of size N to store a null terminated copy of the string_view
 * if string_view is larger than N a char[] will be heap allocated.
 * Note: string_view.data() is _not_ null terminated and can not be passed to functions that expect
 * a c-string
 */
template <size_t N = 120>
class SafeCStr {
public:
    SafeCStr(const std::string_view sv) {
        static_assert(N > 0);
        if (sv.size() < N - 1) {
            std::memcpy(stack.data(), sv.data(), sv.size());
            stack[sv.size()] = 0;
            heap = nullptr;
        } else {
            heap = std::make_unique<char[]>(sv.size() + 1);
            std::memcpy(heap.get(), sv.data(), sv.size());
            heap[sv.size()] = 0;
        }
    }

    const char* c_str() const { return heap ? heap.get() : stack.data(); }
    operator const char*() const { return heap ? heap.get() : stack.data(); }

private:
    std::array<char, N> stack;  // Do not initialize!
    std::unique_ptr<char[]> heap;
};

}  // namespace inviwo
