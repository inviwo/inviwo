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

#pragma once

#include <modules/hdf5/hdf5moduledefine.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/glmvec.h>

#include <warn/push>
#include <warn/ignore/all>
#include <H5Cpp.h>
#include <warn/pop>

#include <modules/hdf5/hdf5types.h>
#include <modules/hdf5/hdf5exception.h>
#include <modules/hdf5/datastructures/hdf5path.h>
#include <modules/hdf5/datastructures/hdf5selection.h>

#include <limits>
#include <memory>
#include <functional>
#include <utility>
#include <ranges>
#include <type_traits>
#include <filesystem>
#include <string>
#include <vector>
#include <ostream>

namespace inviwo {

namespace hdf5 {

/**
 * A small RAII wrapper around an open H5::DataSet that closes it on destruction.
 */
class DataSet : public H5::DataSet {
public:
    explicit DataSet(const H5::DataSet& dataset) : H5::DataSet{dataset} {}
    DataSet(const DataSet&) = delete;
    DataSet& operator=(const DataSet&) = delete;
    DataSet(DataSet&&) = default;
    DataSet& operator=(DataSet&&) = default;
    ~DataSet() override { close(); }
};

class IVW_MODULE_HDF5_API Handle {
public:
    Handle(const std::filesystem::path& filename);
    Handle(const std::filesystem::path& filename, Path path);
    Handle(const Handle& rhs) = default;
    Handle& operator=(const Handle& that) = default;
    Handle(Handle&& rhs) = default;
    Handle& operator=(Handle&& that) = default;

    ~Handle() = default;

    [[nodiscard]] Document getInfo() const;

    [[nodiscard]] const H5::Group& getGroup() const;

    [[nodiscard]] const Path& getPath() const;

    [[nodiscard]] DataSet open() const;

    [[nodiscard]] Handle getHandleForPath(const std::string& path) const;

    /**
     * A lazy range yielding the attributes of this handle's group.
     */
    [[nodiscard]] auto attributes() const {
        return std::views::iota(0, data_.getNumAttrs()) |
               std::views::transform([this](int i) { return data_.openAttribute(i); });
    }

    /**
     * A lazy range yielding the immediate child groups as Handles.
     */
    [[nodiscard]] auto groups() const {
        return std::views::iota(hsize_t{0}, data_.getNumObjs()) |
               std::views::filter(
                   [this](hsize_t i) { return data_.getObjTypeByIdx(i) == H5G_GROUP; }) |
               std::views::transform([this](hsize_t i) {
                   const std::string childName = data_.getObjnameByIdx(i);
                   return Handle{filename_, path_ + childName, data_.openGroup(childName)};
               });
    }

    /**
     * A lazy range yielding the immediate child datasets.
     */
    [[nodiscard]] auto datasets() const {
        return std::views::iota(hsize_t{0}, data_.getNumObjs()) |
               std::views::filter(
                   [this](hsize_t i) { return data_.getObjTypeByIdx(i) == H5G_DATASET; }) |
               std::views::transform([this](hsize_t i) {
                   return DataSet{data_.openDataSet(data_.getObjnameByIdx(i))};
               });
    }

    /**
     * Recursively visit all descendant groups depth-first, invoking @p callback with each Handle.
     */
    template <typename Callback>
    void visitGroups(const Callback& callback) const {
        for (const Handle& child : groups()) {
            callback(child);
            child.visitGroups(callback);
        }
    }

    friend Handle operator+(const Handle& lhs, const Path& rhs) {
        Handle result{lhs};
        result.path_ += rhs;
        return result;
    }
    friend Handle operator+(const Handle& lhs, std::string_view rhs) {
        Handle result{lhs};
        result.path_ += rhs;
        return result;
    }

    static constexpr uvec3 colorCode{101, 101, 188};
    static constexpr std::string_view classIdentifier{"org.inviwo.hdf5.handle"};
    static constexpr std::string_view dataName{"HDF"};

private:
    Handle(std::filesystem::path filename, Path path, const H5::Group& data);

    std::filesystem::path filename_;
    Path path_;
    H5::Group data_;
};

}  // namespace hdf5

}  // namespace inviwo
