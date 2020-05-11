/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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

namespace inviwo {

Document Rasterization::getInfo() const {
    Document doc;
    doc.append("p", "Rasterization functor.");
    return doc;
}

namespace rasterization {

SimpleSpatialEntity TransformSettings::getTransform() const {
    SimpleSpatialEntity transformation;

    switch (*space_) {
        case CoordinateSpace::Model:
            transformation.setModelMatrix(transform_.getMatrix());
            break;

        case CoordinateSpace::World:
        default:
            transformation.setWorldMatrix(transform_.getMatrix());
            break;
    }

    transformation.replaceTransformationOnApply_ = replaceTransform_.get();
    return transformation;
}

SimpleSpatialEntity SimpleSpatialEntity::applyToSpatialEntity(const SpatialEntity<3>& data) const {
    if (replaceTransformationOnApply_) return *this;

    SimpleSpatialEntity result;
    result.setModelMatrix(getModelMatrix() * data.getModelMatrix());
    result.setWorldMatrix(getWorldMatrix() * data.getWorldMatrix());
    return result;
}

TransformSettings::TransformSettings(const std::string& identifier, const std::string& displayName,
                                     InvalidationLevel invalidate)
    : CompositeProperty(identifier, displayName, invalidate)
    , space_(
          "space", "Space",
          {{"model", "Model", CoordinateSpace::Model}, {"world", "World", CoordinateSpace::World}},
          1)
    , replaceTransform_("replace", "Replace Input Transformation", false)
    , transform_("transform", "Transform") {
    addProperty(space_);
    addProperty(replaceTransform_);
    addProperty(transform_.transforms_);
    addProperty(transform_.result_);
}

}  // namespace rasterization

}  // namespace inviwo