/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>

namespace inviwo {

class Mesh;

/**
 * \ingroup properties
 * \brief A CompositeProperty holding properties to show a information about a mesh
 */
class IVW_MODULE_BASE_API MeshInformationProperty : public CompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    MeshInformationProperty(
        std::string identifier, std::string displayName,
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
        PropertySemantics semantics = PropertySemantics::Default);
    MeshInformationProperty(const MeshInformationProperty& rhs);
    virtual MeshInformationProperty* clone() const override;
    virtual ~MeshInformationProperty() = default;

    void updateForNewMesh(const Mesh& mesh);

    // Read-only used to show information
    CompositeProperty defaultMeshInfo_;
    StringProperty defaultDrawType_;
    StringProperty defaultConnectivity_;
    IntSizeTProperty numBuffers_;
    IntSizeTProperty numIndexBuffers_;

    CompositeProperty transformations_;
    FloatMat4Property modelTransform_;
    FloatMat4Property worldTransform_;
    FloatMat3Property basis_;
    FloatVec3Property offset_;

    BoolCompositeProperty meshProperties_;
    FloatVec3Property min_;
    FloatVec3Property max_;
    FloatVec3Property extent_;

    CompositeProperty buffers_;
    CompositeProperty indexBuffers_;

private:
    const size_t maxShownIndexBuffers_ = 15;

    auto props() {
        return std::tie(defaultDrawType_, defaultConnectivity_, numBuffers_, numIndexBuffers_,
                        modelTransform_, worldTransform_, basis_, offset_, min_, max_, extent_);
    }
    auto props() const {
        return std::tie(defaultDrawType_, defaultConnectivity_, numBuffers_, numIndexBuffers_,
                        modelTransform_, worldTransform_, basis_, offset_, min_, max_, extent_);
    }

    auto compositeProps() {
        return std::tie(transformations_, meshProperties_, buffers_, indexBuffers_);
    }
    auto compositeProps() const {
        return std::tie(transformations_, meshProperties_, buffers_, indexBuffers_);
    }
};

}  // namespace inviwo
