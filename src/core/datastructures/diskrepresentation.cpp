/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/core/datastructures/diskrepresentation.h>

namespace inviwo {

DiskRepresentation::DiskRepresentation() : sourceFile_(""), loader_() {}

DiskRepresentation::DiskRepresentation(std::string srcFile, DiskRepresentationLoader* loader)
    : sourceFile_(srcFile), loader_(loader) {}

DiskRepresentation* DiskRepresentation::clone() const { return new DiskRepresentation(*this); }

const std::string& DiskRepresentation::getSourceFile() const { return sourceFile_; }

bool DiskRepresentation::hasSourceFile() const { return !sourceFile_.empty(); }

void DiskRepresentation::setLoader(DiskRepresentationLoader* loader) {
    loader_.reset(loader);
}

std::shared_ptr<DataRepresentation> DiskRepresentation::createRepresentation() const {
    if (!loader_) throw Exception("No loader available to create representation", IvwContext);
    return loader_->createRepresentation();
}

void DiskRepresentation::updateRepresentation(std::shared_ptr<DataRepresentation> dest) const {
    if (!loader_) throw Exception("No loader available to update representation", IvwContext);
    loader_->updateRepresentation(dest);
}

}  // namespace
