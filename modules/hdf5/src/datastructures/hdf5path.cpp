/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

namespace hdf5 {

Path::Path() = default;

Path::Path(const std::string& path) : path_{} { splitString(path); }

Path::Path(const Path& rhs) = default;
Path::Path(Path&& rhs) = default;

Path& Path::operator=(const Path& that) = default;
Path& Path::operator=(Path&& that) = default;

Path& Path::push(const std::string& path) {
    splitString(path);
    return *this;
}
Path& Path::push(const Path& rhs) {
    for (auto elem : rhs.path_) {
        path_.push_back(elem);
    }
    return *this;
}
Path& Path::pop() {
    path_.pop_back();
    return *this;
}

Path& Path::operator+=(const Path& rhs) {
    for (auto elem : rhs.path_) {
        path_.push_back(elem);
    }
    return *this;
}
Path& Path::operator+=(const std::string& path) {
    splitString(path);
    return *this;
}

Path::operator std::string() const { return toString(); }

std::string Path::toString() const { return "/" + joinString(path_, "/"); }

void Path::splitString(const std::string& string) {
    std::stringstream data(string);

    std::string line;
    while (std::getline(data, line, '/')) {
        if (!line.empty()) path_.push_back(line);
    }
}

Path operator+(const Path& lhs, const Path& rhs) {
    Path newpath(lhs);
    newpath.push(rhs);
    return newpath;
}

Path operator+(const Path& lhs, const std::string& rhs) {
    Path newpath(lhs);
    newpath.push(rhs);
    return newpath;
}

}  // namespace hdf5

}  // namespace inviwo
