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
#pragma once

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/properties/boolcompositeproperty.h>  // for BoolCompositeProperty
#include <inviwo/core/properties/compositeproperty.h>      // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>      // for InvalidationLevel, Invalidatio...
#include <inviwo/core/properties/ordinalproperty.h>        // for FloatVec3Property, FloatMat4Pr...
#include <inviwo/core/properties/propertysemantics.h>      // for PropertySemantics, PropertySem...
#include <inviwo/core/properties/stringproperty.h>         // for StringProperty

#include <cstddef>      // for size_t
#include <string>       // for string
#include <string_view>  // for string_view
#include <tuple>        // for tie

namespace inviwo {

class Mesh;

/**
 * \ingroup properties
 * \brief A CompositeProperty holding properties to show information about a mesh and its buffers
 */
class IVW_MODULE_BASE_API MeshInformationProperty : public CompositeProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.MeshInformationProperty"};

    MeshInformationProperty(
        std::string_view identifier, std::string_view displayName,
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
    std::array<FloatMat4Property, 6> spaceTransforms_;

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
