/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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

#include <modules/hdf5/datastructures/hdf5handle.h>

namespace inviwo {

namespace hdf5 {

namespace {
H5::Group load(const std::filesystem::path& filename, const std::string& path) {
    H5::H5File hdfFile(filename.generic_string(), H5F_ACC_RDONLY);
    return hdfFile.openGroup(path);
}
}  // namespace

Handle::Handle(const std::filesystem::path& filename)
    : filename_(filename), path_("/"), data_{load(filename_, path_)} {}

Handle::Handle(const std::filesystem::path& filename, Path path)
    : filename_(filename), path_(path), data_{load(filename_, path_)} {}

Handle::Handle(std::filesystem::path filename, Path path, const H5::Group& data)
    : filename_(std::move(filename)), path_(std::move(path)), data_(data) {}

Handle Handle::getHandleForPath(const std::string& path) const {
    const Path newPath = path_ + path;
    return Handle{filename_, newPath, data_.openGroup(newPath)};
}

Document Handle::getInfo() const {
    Document doc;
    doc.append("p", "File: " + filename_.generic_string() + path_.toString());
    return doc;
}

const H5::Group& Handle::getGroup() const { return data_; }

const Path& Handle::getPath() const { return path_; }

DataSet Handle::open() const { return DataSet{data_.openDataSet(path_)}; }

}  // namespace hdf5

}  // namespace inviwo
