/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <modules/meshrenderinggl/datastructures/rasterization.h>

#include <inviwo/core/util/document.h>  // for Document

namespace inviwo {

RasterizationProcessor::RasterizationProcessor(std::string_view identifier,
                                               std::string_view displayName)
    : Processor(identifier, displayName), outport_{"image"}, needsInitialize_{false} {
    addPort(outport_);
}

void RasterizationProcessor::initializeResources() { needsInitialize_ = true; }

void RasterizationProcessor::process() {
    auto rasterization = std::make_shared<Rasterization>(
        std::dynamic_pointer_cast<RasterizationProcessor>(shared_from_this()));
    outport_.setData(rasterization);
}

Document Rasterization::getInfo() const {
    if (auto p = getProcessor()) {
        return p->getInfo();
    } else {
        Document doc;
        doc.append("p", "Rasterization functor.");
        return doc;
    }
}

Rasterization::Rasterization(std::shared_ptr<RasterizationProcessor> processor)
    : processor_{processor} {}

std::shared_ptr<RasterizationProcessor> Rasterization::getProcessor() const {
    return processor_.lock();
}

bool Rasterization::usesFragmentLists() const {
    if (auto p = getProcessor()) {
        return p->usesFragmentLists();
    } else {
        return false;
    }
}

std::optional<mat4> Rasterization::boundingBox() const {
    if (auto p = getProcessor()) {
        return p->boundingBox();
    } else {
        return std::nullopt;
    }
}

}  // namespace inviwo
