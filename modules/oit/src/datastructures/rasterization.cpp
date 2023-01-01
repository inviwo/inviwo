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

#include <modules/oit/datastructures/rasterization.h>
#include <modules/oit/processors/rasterizer.h>

#include <inviwo/core/util/document.h>  // for Document

namespace inviwo {

Document Rasterization::getInfo() const {
    if (auto p = getProcessor()) {
        return p->getInfo();
    } else {
        Document doc;
        doc.append("p", "Rasterization functor.");
        return doc;
    }
}

Rasterization::Rasterization(std::shared_ptr<Rasterizer> processor) : processor_{processor} {}

std::shared_ptr<Rasterizer> Rasterization::getProcessor() const { return processor_.lock(); }

void Rasterization::rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform) const {
    if (auto rp = getProcessor()) {
        rp->rasterize(imageSize, worldMatrixTransform);
    }
}

UseFragmentList Rasterization::usesFragmentLists() const {
    if (auto p = getProcessor()) {
        return p->usesFragmentLists();
    } else {
        return UseFragmentList::No;
    }
}

std::optional<mat4> Rasterization::boundingBox() const {
    if (auto p = getProcessor()) {
        return p->boundingBox();
    } else {
        return std::nullopt;
    }
}

auto Rasterization::getRaycastingState() const -> std::optional<RaycastingState> {
    if (auto p = getProcessor()) {
        return p->getRaycastingState();
    } else {
        return std::nullopt;
    }
}

}  // namespace inviwo
