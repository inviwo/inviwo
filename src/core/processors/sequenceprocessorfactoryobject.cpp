/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/processors/sequenceprocessorfactoryobject.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/serialization/serialization.h>

namespace inviwo {

SequenceProcessorFactoryObject::SequenceProcessorFactoryObject(const std::filesystem::path& file)
    : ProcessorFactoryObject(makeProcessorInfo(file), "inviwo::SequenceProcessor"), file_{file} {}

std::shared_ptr<Processor> SequenceProcessorFactoryObject::create(InviwoApplication* app) const {
    auto pi = getProcessorInfo();
    return std::make_shared<SequenceProcessor>(pi.displayName, pi.displayName, app, file_);
}

Document SequenceProcessorFactoryObject::getMetaInformation() const {
    return Document(file_.string());
}

ProcessorInfo SequenceProcessorFactoryObject::makeProcessorInfo(const std::filesystem::path& file) {

    auto pi = ProcessorTraits<SequenceProcessor>::getProcessorInfo();
    auto name = file.stem().string();
    auto id = pi.classIdentifier + util::stripIdentifier(file.string());

    Deserializer d{file};
    std::string tags;
    d.deserialize("DisplayName", name);
    d.deserialize("Tags", tags);

    return {
        id,                 // Class identifier
        name,               // Display name
        "Sequences",        // Category
        CodeState::Stable,  // Code state
        tags,               // Tags
    };
}

}  // namespace inviwo
