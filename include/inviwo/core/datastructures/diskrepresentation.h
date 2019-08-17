/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_DISKREPRESENTATION_H
#define IVW_DISKREPRESENTATION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/cloneableptr.h>

namespace inviwo {

template <typename Repr>
class DiskRepresentationLoader;

/**
 * \ingroup datastructures
 * Base class for all DiskRepresentations \see Data, DataRepresentation
 */
template <typename Repr>
class DiskRepresentation {
public:
    DiskRepresentation() = default;
    DiskRepresentation(const std::string& srcFile,
                       DiskRepresentationLoader<Repr>* loader = nullptr);
    DiskRepresentation(const DiskRepresentation& rhs) = default;
    DiskRepresentation& operator=(const DiskRepresentation& that) = default;
    virtual ~DiskRepresentation() = default;
    virtual DiskRepresentation* clone() const;

    const std::string& getSourceFile() const;
    bool hasSourceFile() const;

    void setLoader(DiskRepresentationLoader<Repr>* loader);

    std::shared_ptr<Repr> createRepresentation() const;
    void updateRepresentation(std::shared_ptr<Repr> dest) const;

private:
    std::string sourceFile_;

    // DiskRepresentation owns a DataReader to be able to convert itself into RAM.
    util::cloneable_ptr<DiskRepresentationLoader<Repr>> loader_;
};

template <typename Repr>
DiskRepresentation<Repr>::DiskRepresentation(const std::string& srcFile,
                                             DiskRepresentationLoader<Repr>* loader)
    : sourceFile_(srcFile), loader_(loader) {}

template <typename Repr>
DiskRepresentation<Repr>* DiskRepresentation<Repr>::clone() const {
    return new DiskRepresentation<Repr>(*this);
}

template <typename Repr>
const std::string& DiskRepresentation<Repr>::getSourceFile() const {
    return sourceFile_;
}

template <typename Repr>
bool DiskRepresentation<Repr>::hasSourceFile() const {
    return !sourceFile_.empty();
}

template <typename Repr>
void DiskRepresentation<Repr>::setLoader(DiskRepresentationLoader<Repr>* loader) {
    loader_.reset(loader);
}

template <typename Repr>
std::shared_ptr<Repr> DiskRepresentation<Repr>::createRepresentation() const {
    if (!loader_) throw Exception("No loader available to create representation", IVW_CONTEXT);
    return loader_->createRepresentation();
}

template <typename Repr>
void DiskRepresentation<Repr>::updateRepresentation(std::shared_ptr<Repr> dest) const {
    if (!loader_) throw Exception("No loader available to update representation", IVW_CONTEXT);
    loader_->updateRepresentation(dest);
}

template <typename Repr>
class DiskRepresentationLoader {
public:
    virtual ~DiskRepresentationLoader() = default;
    virtual DiskRepresentationLoader* clone() const = 0;
    virtual std::shared_ptr<Repr> createRepresentation() const = 0;
    virtual void updateRepresentation(std::shared_ptr<Repr> dest) const = 0;
};

}  // namespace inviwo

#endif  // IVW_DISKREPRESENTATION_H
