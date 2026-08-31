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

#include <modules/hdf5/datastructures/hdf5path.h>

#include <ranges>
#include <string_view>

namespace inviwo {

namespace hdf5 {

Path::Path() : path_{"/"} {}

Path::Path(std::string_view path) : path_{"/"} { append(path); }

Path& Path::push(std::string_view path) {
    append(path);
    return *this;
}
Path& Path::push(const Path& rhs) {
    append(rhs.path_);
    return *this;
}
Path& Path::pop() {
    if (const auto pos = path_.rfind('/'); pos == 0) {
        path_ = "/";
    } else if (pos != std::string::npos) {
        path_.erase(pos);
    }
    return *this;
}

Path& Path::operator+=(const Path& rhs) {
    append(rhs.path_);
    return *this;
}
Path& Path::operator+=(std::string_view path) {
    append(path);
    return *this;
}

Path::operator const std::string&() const { return path_; }

const std::string& Path::toString() const { return path_; }

void Path::append(std::string_view path) {
    for (const auto part : std::views::split(path, '/')) {
        if (part.empty()) continue;
        if (path_.size() != 1) path_.push_back('/');
        path_.append(part.begin(), part.end());
    }
}

}  // namespace hdf5

}  // namespace inviwo
