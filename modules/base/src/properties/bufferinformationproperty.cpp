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

#include <modules/base/properties/bufferinformationproperty.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

const std::string BufferInformationProperty::classIdentifier =
    "org.inviwo.BufferInformationProperty";
std::string BufferInformationProperty::getClassIdentifier() const { return classIdentifier; }

const std::string MeshBufferInformationProperty::classIdentifier =
    "org.inviwo.MeshBufferInformationProperty";
std::string MeshBufferInformationProperty::getClassIdentifier() const { return classIdentifier; }

const std::string IndexBufferInformationProperty::classIdentifier =
    "org.inviwo.IndexBufferInformationProperty";
std::string IndexBufferInformationProperty::getClassIdentifier() const { return classIdentifier; }

BufferInformationProperty::BufferInformationProperty(std::string identifier,
                                                     std::string displayName,
                                                     InvalidationLevel invalidationLevel,
                                                     PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , format_("format", "Format", "")
    , size_("size", "Size", 0, 0, std::numeric_limits<size_t>::max(), 1,
            InvalidationLevel::InvalidOutput, PropertySemantics("Text"))
    , usage_("usage", "Usage")
    , target_("target", "Target") {

    util::for_each_in_tuple(
        [&](auto& e) {
            e.setReadOnly(true);
            e.setSerializationMode(PropertySerializationMode::None);
            e.setCurrentStateAsDefault();
            this->addProperty(e);
        },
        props());
}

BufferInformationProperty::BufferInformationProperty(const BufferInformationProperty& rhs)
    : CompositeProperty(rhs)
    , format_(rhs.format_)
    , size_(rhs.size_)
    , usage_(rhs.usage_)
    , target_(rhs.target_) {
    util::for_each_in_tuple([&](auto& e) { this->addProperty(e); }, props());
}

BufferInformationProperty* BufferInformationProperty::clone() const {
    return new BufferInformationProperty(*this);
}

void BufferInformationProperty::updateFromBuffer(const BufferBase& buffer) {
    format_.set(buffer.getDataFormat()->getString());
    size_.set(buffer.getSize());
    usage_.set(toString(buffer.getBufferUsage()));
    target_.set(toString(buffer.getBufferTarget()));
}

MeshBufferInformationProperty::MeshBufferInformationProperty(std::string identifier,
                                                             std::string displayName,
                                                             InvalidationLevel invalidationLevel,
                                                             PropertySemantics semantics)
    : BufferInformationProperty(identifier, displayName, invalidationLevel, semantics)
    , type_("type", "Type")
    , location_("location", "Location", -1, std::numeric_limits<int>::min(),
                std::numeric_limits<int>::max(), 1, InvalidationLevel::InvalidOutput,
                PropertySemantics("Text")) {

    target_.setVisible(false);

    util::for_each_in_tuple(
        [this, i = 0](auto& e) mutable {
            e.setReadOnly(true);
            e.setSerializationMode(PropertySerializationMode::None);
            e.setCurrentStateAsDefault();
            this->insertProperty(i++, e);
        },
        props());
}

MeshBufferInformationProperty::MeshBufferInformationProperty(
    const MeshBufferInformationProperty& rhs)
    : BufferInformationProperty(rhs), type_(rhs.type_), location_(rhs.location_) {
    util::for_each_in_tuple([&, i = 0](auto& e) mutable { this->insertProperty(i++, e); }, props());
}

MeshBufferInformationProperty* MeshBufferInformationProperty::clone() const {
    return new MeshBufferInformationProperty(*this);
}

void MeshBufferInformationProperty::updateFromBuffer(Mesh::BufferInfo info,
                                                     const BufferBase& buffer) {
    BufferInformationProperty::updateFromBuffer(buffer);
    type_.set(toString(info.type));
    location_.set(info.location);
}

IndexBufferInformationProperty::IndexBufferInformationProperty(std::string identifier,
                                                               std::string displayName,
                                                               InvalidationLevel invalidationLevel,
                                                               PropertySemantics semantics)
    : BufferInformationProperty(identifier, displayName, invalidationLevel, semantics)
    , drawType_("defaultDrawType", "Draw Type")
    , connectivity_("defaultConnectivity", "Connectivity") {

    format_.setVisible(false);
    usage_.setVisible(false);
    target_.setVisible(false);

    util::for_each_in_tuple(
        [this, i = 0](auto& e) mutable {
            e.setReadOnly(true);
            e.setSerializationMode(PropertySerializationMode::None);
            e.setCurrentStateAsDefault();
            this->insertProperty(i++, e);
        },
        props());
}

IndexBufferInformationProperty::IndexBufferInformationProperty(
    const IndexBufferInformationProperty& rhs)
    : BufferInformationProperty(rhs), drawType_(rhs.drawType_), connectivity_(rhs.connectivity_) {
    util::for_each_in_tuple([&, i = 0](auto& e) mutable { this->insertProperty(i++, e); }, props());
}

IndexBufferInformationProperty* IndexBufferInformationProperty::clone() const {
    return new IndexBufferInformationProperty(*this);
}

void IndexBufferInformationProperty::updateFromBuffer(Mesh::MeshInfo info,
                                                      const BufferBase& buffer) {
    BufferInformationProperty::updateFromBuffer(buffer);
    drawType_.set(toString(info.dt));
    connectivity_.set(toString(info.ct));
}

}  // namespace inviwo
