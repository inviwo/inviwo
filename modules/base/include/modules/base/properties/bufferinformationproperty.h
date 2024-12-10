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

#include <inviwo/core/datastructures/geometry/mesh.h>  // for Mesh, Mesh::BufferInfo
#include <inviwo/core/properties/compositeproperty.h>  // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel, InvalidationLev...
#include <inviwo/core/properties/ordinalproperty.h>    // for IntProperty, IntSizeTProperty
#include <inviwo/core/properties/propertysemantics.h>  // for PropertySemantics, PropertySemanti...
#include <inviwo/core/properties/stringproperty.h>     // for StringProperty

#include <string>       // for string
#include <string_view>  // for string_view
#include <tuple>        // for tie

namespace inviwo {

class BufferBase;

/**
 * \ingroup properties
 * \brief A CompositeProperty holding properties to show a information about an Inviwo Buffer
 */
class IVW_MODULE_BASE_API BufferInformationProperty : public CompositeProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.BufferInformationProperty"};

    BufferInformationProperty(
        std::string_view identifier, std::string_view displayName,
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
        PropertySemantics semantics = PropertySemantics::Default);
    BufferInformationProperty(const BufferInformationProperty& rhs);
    virtual BufferInformationProperty* clone() const override;
    virtual ~BufferInformationProperty() = default;

    void updateFromBuffer(const BufferBase& buffer);

    StringProperty format_;
    IntSizeTProperty size_;
    StringProperty usage_;
    StringProperty target_;

private:
    auto props() { return std::tie(format_, size_, usage_, target_); }
    auto props() const { return std::tie(format_, size_, usage_, target_); }
};

class IVW_MODULE_BASE_API MeshBufferInformationProperty : public BufferInformationProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.MeshBufferInformationProperty"};

    MeshBufferInformationProperty(
        std::string_view identifier, std::string_view displayName,
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
        PropertySemantics semantics = PropertySemantics::Default);
    MeshBufferInformationProperty(const MeshBufferInformationProperty& rhs);
    virtual MeshBufferInformationProperty* clone() const override;
    virtual ~MeshBufferInformationProperty() = default;

    void updateFromBuffer(Mesh::BufferInfo info, const BufferBase& buffer);

    StringProperty type_;
    IntProperty location_;

private:
    auto props() { return std::tie(type_, location_); }
    auto props() const { return std::tie(type_, location_); }
};

class IVW_MODULE_BASE_API IndexBufferInformationProperty : public BufferInformationProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.IndexBufferInformationProperty"};

    IndexBufferInformationProperty(
        std::string_view identifier, std::string_view displayName,
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
        PropertySemantics semantics = PropertySemantics::Default);
    IndexBufferInformationProperty(const IndexBufferInformationProperty& rhs);
    virtual IndexBufferInformationProperty* clone() const override;
    virtual ~IndexBufferInformationProperty() = default;

    void updateFromBuffer(Mesh::MeshInfo info, const BufferBase& buffer);

    StringProperty drawType_;
    StringProperty connectivity_;

private:
    auto props() { return std::tie(drawType_, connectivity_); }
    auto props() const { return std::tie(drawType_, connectivity_); }
};

}  // namespace inviwo
