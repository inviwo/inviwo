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

#include <modules/base/properties/meshinformationproperty.h>

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/util/stringconversion.h>
#include <modules/base/properties/bufferinformationproperty.h>
#include <modules/base/algorithm/dataminmax.h>

namespace inviwo {

const std::string MeshInformationProperty::classIdentifier = "org.inviwo.MeshInformationProperty";
std::string MeshInformationProperty::getClassIdentifier() const { return classIdentifier; }

MeshInformationProperty::MeshInformationProperty(std::string identifier, std::string displayName,
                                                 InvalidationLevel invalidationLevel,
                                                 PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , defaultMeshInfo_("defaultMeshInfo", "Default Mesh Info")
    , defaultDrawType_("defaultDrawType", "Draw Type")
    , defaultConnectivity_("defaultConnectivity", "Connectivity")
    , numBuffers_("numBuffers", "Number of Buffers", 0, 0, std::numeric_limits<size_t>::max(), 1,
                  InvalidationLevel::Valid, PropertySemantics::Text)
    , numIndexBuffers_("numIndexBuffers", "Number of Index Buffers", 0, 0,
                       std::numeric_limits<size_t>::max(), 1, InvalidationLevel::Valid,
                       PropertySemantics::Text)

    , transformations_("transformations", "Transformations")
    , modelTransform_("modelTransform_", "Model Transform", mat4(1.0f),
                      util::filled<mat3>(std::numeric_limits<float>::lowest()),
                      util::filled<mat3>(std::numeric_limits<float>::max()),
                      util::filled<mat3>(0.001f), InvalidationLevel::Valid)
    , worldTransform_("worldTransform_", "World Transform", mat4(1.0f),
                      util::filled<mat3>(std::numeric_limits<float>::lowest()),
                      util::filled<mat3>(std::numeric_limits<float>::max()),
                      util::filled<mat3>(0.001f), InvalidationLevel::Valid)
    , basis_("basis", "Basis", mat3(1.0f), util::filled<mat3>(std::numeric_limits<float>::lowest()),
             util::filled<mat3>(std::numeric_limits<float>::max()), util::filled<mat3>(0.001f),
             InvalidationLevel::Valid)
    , offset_("offset", "Offset", vec3(0.0f), vec3(std::numeric_limits<float>::lowest()),
              vec3(std::numeric_limits<float>::max()), vec3(0.001f), InvalidationLevel::Valid,
              PropertySemantics::Text)

    , meshProperties_("meshProperties", "Mesh Properties", false)
    , min_("minPos", "Minimum Pos", vec3{0.0f}, vec3{std::numeric_limits<float>::lowest()},
           vec3{std::numeric_limits<float>::max()}, vec3{0.01f}, InvalidationLevel::Valid,
           PropertySemantics::Text)
    , max_("maxPos", "Maximum Pos", vec3{0.0f}, vec3{std::numeric_limits<float>::lowest()},
           vec3{std::numeric_limits<float>::max()}, vec3{0.01f}, InvalidationLevel::Valid,
           PropertySemantics::Text)
    , extent_("extent", "Extent", vec3{0.0f}, vec3{std::numeric_limits<float>::lowest()},
              vec3{std::numeric_limits<float>::max()}, vec3{0.01f}, InvalidationLevel::Valid,
              PropertySemantics::Text)

    , buffers_("buffers", "Buffers")
    , indexBuffers_("indexBuffers", "Index Buffers") {

    util::for_each_in_tuple(
        [](auto& e) {
            e.setReadOnly(true);
            e.setSerializationMode(PropertySerializationMode::None);
            e.setCurrentStateAsDefault();
        },
        props());

    defaultMeshInfo_.addProperties(defaultDrawType_, defaultConnectivity_);
    transformations_.addProperties(modelTransform_, worldTransform_, basis_, offset_);
    meshProperties_.addProperties(min_, max_, extent_);
    addProperties(defaultMeshInfo_, numBuffers_, numIndexBuffers_, transformations_,
                  meshProperties_, buffers_, indexBuffers_);

    util::for_each_in_tuple(
        [](auto& e) {
            e.setCollapsed(true);
            e.setCurrentStateAsDefault();
        },
        compositeProps());
}

MeshInformationProperty::MeshInformationProperty(const MeshInformationProperty& rhs)
    : CompositeProperty(rhs)
    , defaultMeshInfo_(rhs.defaultMeshInfo_)
    , defaultDrawType_(rhs.defaultDrawType_)
    , defaultConnectivity_(rhs.defaultConnectivity_)
    , numBuffers_(rhs.numBuffers_)
    , numIndexBuffers_(rhs.numIndexBuffers_)
    , transformations_(rhs.transformations_)
    , modelTransform_(rhs.modelTransform_)
    , worldTransform_(rhs.worldTransform_)
    , basis_(rhs.basis_)
    , offset_(rhs.offset_)
    , meshProperties_(rhs.meshProperties_)
    , min_(rhs.min_)
    , max_(rhs.max_)
    , extent_(rhs.extent_)
    , buffers_(rhs.buffers_)
    , indexBuffers_(rhs.indexBuffers_) {

    defaultMeshInfo_.addProperties(defaultDrawType_, defaultConnectivity_);
    transformations_.addProperties(modelTransform_, worldTransform_, basis_, offset_);
    meshProperties_.addProperties(min_, max_, extent_);
    addProperties(defaultMeshInfo_, numBuffers_, numIndexBuffers_, transformations_,
                  meshProperties_, buffers_, indexBuffers_);
}

MeshInformationProperty* MeshInformationProperty::clone() const {
    return new MeshInformationProperty(*this);
}

namespace detail {

template <typename T>
void allocateBufferProps(CompositeProperty* composite, const std::string& nameprefix,
                         size_t count) {
    while (composite->size() < count) {
        auto p = std::make_unique<T>(nameprefix + std::to_string(composite->size()), "");
        p->setReadOnly(true);
        composite->addProperty(p.release());
    }
    // remove superfluous child properties
    while (composite->size() > count) {
        composite->removeProperty(composite->size() - 1);
    }
}

}  // namespace detail

void MeshInformationProperty::updateForNewMesh(const Mesh& mesh) {
    const auto meshInfo = mesh.getDefaultMeshInfo();
    defaultDrawType_.set(toString(meshInfo.dt));
    defaultConnectivity_.set(toString(meshInfo.ct));

    const auto numBuffers = mesh.getNumberOfBuffers();

    numBuffers_.set(numBuffers);
    numIndexBuffers_.set(mesh.getNumberOfIndicies());

    // transformations
    modelTransform_.set(mesh.getModelMatrix());
    worldTransform_.set(mesh.getWorldMatrix());
    basis_.set(mesh.getBasis());
    offset_.set(mesh.getOffset());

    // general mesh information
    if (meshProperties_.isChecked()) {
        auto posbuffer = mesh.findBuffer(BufferType::PositionAttrib);
        if (posbuffer.first) {
            auto minmax = util::bufferMinMax(posbuffer.first);
            min_.set(minmax.first);
            max_.set(minmax.second);
            extent_.set(minmax.second - minmax.first);
        } else {
            min_.resetToDefaultState();
            max_.resetToDefaultState();
            extent_.resetToDefaultState();
        }
    }

    // update buffer information
    detail::allocateBufferProps<MeshBufferInformationProperty>(&buffers_, "meshbuffer", numBuffers);
    const auto numIndexBuffers = std::min(mesh.getNumberOfIndicies(), maxShownIndexBuffers_);
    detail::allocateBufferProps<IndexBufferInformationProperty>(&indexBuffers_, "indexbuffer",
                                                                numIndexBuffers);

    auto updateBufferProperty = [](auto* prop, const auto& elem, const std::string& name) {
        prop->setDisplayName(name);
        prop->updateFromBuffer(elem.first, *(elem.second.get()));
    };

    // update mesh buffer information
    const auto& bufferprops = buffers_.getProperties();
    const auto& buffers = mesh.getBuffers();
    for (size_t i = 0; i < numBuffers; ++i) {
        updateBufferProperty(static_cast<MeshBufferInformationProperty*>(bufferprops[i]),
                             buffers[i], "Buffer " + std::to_string(i + 1));
    }

    // update index buffer information
    const auto& indexbufferprops = indexBuffers_.getProperties();
    const auto& indices = mesh.getIndexBuffers();
    for (size_t i = 0; i < numIndexBuffers; ++i) {
        updateBufferProperty(static_cast<IndexBufferInformationProperty*>(indexbufferprops[i]),
                             indices[i], "Index Buffer " + std::to_string(i + 1));
    }

    util::for_each_in_tuple([&](auto& e) { e.setCurrentStateAsDefault(); }, props());
}

}  // namespace inviwo
