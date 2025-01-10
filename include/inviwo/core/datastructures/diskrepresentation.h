/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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
#include <inviwo/core/util/cloneableptr.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>

#include <string>
#include <string_view>
#include <memory>
#include <filesystem>

#include <fmt/std.h>

namespace inviwo {

template <typename Repr>
class DiskRepresentationLoader;

/**
 * \ingroup datastructures
 * Base class for all DiskRepresentations \see Data, DataRepresentation
 */
template <typename Repr, typename Self>
class DiskRepresentation {
public:
    DiskRepresentation() = default;
    DiskRepresentation(const std::filesystem::path& srcFile,
                       DiskRepresentationLoader<Repr>* loader = nullptr);
    DiskRepresentation(const DiskRepresentation& rhs) = default;
    DiskRepresentation& operator=(const DiskRepresentation& that) = default;
    virtual ~DiskRepresentation() = default;
    virtual DiskRepresentation* clone() const;

    const std::filesystem::path& getSourceFile() const;
    bool hasSourceFile() const;

    void setLoader(DiskRepresentationLoader<Repr>* loader);

    std::shared_ptr<Repr> createRepresentation() const;
    void updateRepresentation(std::shared_ptr<Repr> dest) const;

private:
    std::filesystem::path sourceFile_;

    // DiskRepresentation owns a DataReader to be able to convert itself into RAM.
    util::cloneable_ptr<DiskRepresentationLoader<Repr>> loader_;
};

template <typename Repr, typename Self>
DiskRepresentation<Repr, Self>::DiskRepresentation(const std::filesystem::path& srcFile,
                                                   DiskRepresentationLoader<Repr>* loader)
    : sourceFile_(srcFile), loader_(loader) {}

template <typename Repr, typename Self>
DiskRepresentation<Repr, Self>* DiskRepresentation<Repr, Self>::clone() const {
    return new DiskRepresentation<Repr, Self>(*this);
}

template <typename Repr, typename Self>
const std::filesystem::path& DiskRepresentation<Repr, Self>::getSourceFile() const {
    return sourceFile_;
}

template <typename Repr, typename Self>
bool DiskRepresentation<Repr, Self>::hasSourceFile() const {
    return !sourceFile_.empty();
}

template <typename Repr, typename Self>
void DiskRepresentation<Repr, Self>::setLoader(DiskRepresentationLoader<Repr>* loader) {
    loader_.reset(loader);
}

template <typename Repr, typename Self>
std::shared_ptr<Repr> DiskRepresentation<Repr, Self>::createRepresentation() const {
    if (!loader_) throw Exception("No loader available to create representation");
    return loader_->createRepresentation(*static_cast<const Self*>(this));
}

template <typename Repr, typename Self>
void DiskRepresentation<Repr, Self>::updateRepresentation(std::shared_ptr<Repr> dest) const {
    if (!loader_) throw Exception("No loader available to update representation");
    loader_->updateRepresentation(dest, *static_cast<const Self*>(this));
}

template <typename Repr>
class DiskRepresentationLoader {
public:
    virtual ~DiskRepresentationLoader() = default;
    virtual DiskRepresentationLoader* clone() const = 0;
    virtual std::shared_ptr<Repr> createRepresentation(const Repr&) const = 0;
    virtual void updateRepresentation(std::shared_ptr<Repr> dest, const Repr&) const = 0;

    static std::filesystem::path findFile(const std::filesystem::path& path) {
        if (std::filesystem::is_regular_file(path)) {
            return path;
        } else if (const auto newPath = filesystem::addBasePath(path);
                   std::filesystem::is_regular_file(newPath)) {
            return newPath;
        } else {
            throw DataReaderException(SourceContext{}, "Error could not find input file: {}", path);
        }
    }
};

}  // namespace inviwo
