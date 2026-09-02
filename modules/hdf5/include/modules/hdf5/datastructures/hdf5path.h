/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2026 Inviwo Foundation
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

#include <modules/hdf5/hdf5moduledefine.h>

#include <string>
#include <string_view>

namespace inviwo {

namespace hdf5 {

/**
 * A normalized HDF5 group path, e.g. `/group/subgroup/dataset`. The path is stored as a single
 * normalized string with a leading `/`, no trailing `/` and no empty segments. An empty path
 * represents the root `/`.
 */
class IVW_MODULE_HDF5_API Path {
public:
    Path();
    explicit Path(std::string_view path);
    Path(const Path& rhs) = default;
    Path& operator=(const Path& that) = default;
    Path(Path&& rhs) = default;
    Path& operator=(Path&& that) = default;

    Path& push(std::string_view path);
    Path& push(const Path& path);
    Path& pop();

    Path& operator+=(const Path& path);
    Path& operator+=(std::string_view path);

    operator const std::string&() const;

    [[nodiscard]] const std::string& toString() const;

    friend Path operator+(const Path& lhs, const Path& rhs) {
        Path result{lhs};
        result.push(rhs);
        return result;
    }
    friend Path operator+(const Path& lhs, std::string_view rhs) {
        Path result{lhs};
        result.push(rhs);
        return result;
    }

private:
    void append(std::string_view path);
    std::string path_;
};

}  // namespace hdf5

}  // namespace inviwo
