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

#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/util/glmvec.h>

#include <functional>

namespace inviwo::grid {

enum class Part : std::uint8_t { First, Mid, Last };

template <typename Func>
void loop(size_t N, Func func) {
    func.template operator()<Part::First>(size_t{0});
    for (size_t i = 1; i < N - 1; ++i) {
        func.template operator()<Part::Mid>(i);
    }
    func.template operator()<Part::Last>(N - 1);
}

template <typename Func>
void loop(size3_t dims, Func func, std::function<void(double)> progress = nullptr,
          std::function<bool()> stop = nullptr) {

    loop(dims.z, [&]<Part Pz>(size_t z) {
        if (stop && stop()) return;
        if (progress) progress(static_cast<double>(z) / static_cast<double>(dims.z));
        loop(dims.y, [&]<Part Py>(size_t y) {
            loop(dims.x, [&]<Part Px>(size_t x) { func.template operator()<Px, Py, Pz>(x, y, z); });
        });
    });
}

template <Part part, Wrapping wrap>
size_t prev(size_t i, size_t N) {
    if constexpr (part == Part::First) {
        if constexpr (wrap == Wrapping::Clamp) {
            return size_t{0};
        } else if constexpr (wrap == Wrapping::Repeat) {
            return N - 1;
        } else {  // Wrapping::Mirror
            return size_t{1};
        }
    } else if constexpr (part == Part::Mid) {
        return i - 1;
    } else {
        return N - 2;
    }
}
template <Part part, Wrapping wrap>
size_t next(size_t i, size_t N) {
    if constexpr (part == Part::First) {
        return size_t{1};
    } else if constexpr (part == Part::Mid) {
        return i + 1;
    } else {
        if constexpr (wrap == Wrapping::Clamp) {
            return N - 1;
        } else if constexpr (wrap == Wrapping::Repeat) {
            return size_t{1};
        } else {
            return N - 2;
        }
    }
}

template <Part P, Wrapping W>
double invStep() {
    if constexpr (W == Wrapping::Clamp && P != Part::Mid) {
        return 1.0;
    } else {
        return 0.5;
    }
}

}  // namespace inviwo::grid
