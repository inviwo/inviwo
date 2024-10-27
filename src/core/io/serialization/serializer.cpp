/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#include <inviwo/core/io/serialization/serializer.h>

#include <inviwo/core/util/exception.h>
#include <inviwo/core/io/serialization/ticpp.h>
#include <inviwo/core/io/serialization/serializationexception.h>
#include <inviwo/core/util/safecstr.h>

namespace inviwo {

Serializer::Serializer(const std::filesystem::path& fileName, allocator_type alloc)
    : SerializeBase(fileName, alloc), buffer{alloc} {
    try {
        auto decl = alloc.new_object<TiXmlDeclaration>(SerializeConstants::XmlVersion, "UTF-8", "");
        doc_->LinkEndChild(decl);
        rootElement_ = alloc.new_object<TiXmlElement>(SerializeConstants::InviwoWorkspace);
        rootElement_->SetAttribute(
            SerializeConstants::VersionAttribute,
            detail::toStr(SerializeConstants::InviwoWorkspaceVersion, buffer));
        doc_->LinkEndChild(rootElement_);

    } catch (const TiXmlError& e) {
        throw SerializationException(e.what(), IVW_CONTEXT);
    }
}

Serializer::~Serializer() {}

void Serializer::serialize(std::string_view key, const std::filesystem::path& path,
                           const SerializationTarget& target) {

    if (target == SerializationTarget::Attribute) {
        setAttribute(rootElement_, key, path.generic_string());
    } else {
        auto nodeSwitch = switchToNewNode(key);
        setAttribute(rootElement_, SerializeConstants::ContentAttribute, path.generic_string());
    }
}

void Serializer::serialize(std::string_view key, const Serializable& sObj) {
    auto alloc = doc_->getAllocator();
    NodeSwitch nodeSwitch{
        *this, rootElement_->LinkEndChild(alloc.new_object<TiXmlElement>(key))->ToElement()};
    sObj.serialize(*this);
}

NodeSwitch Serializer::switchToNewNode(std::string_view key) {
    auto alloc = doc_->getAllocator();

    return {*this, rootElement_->LinkEndChild(alloc.new_object<TiXmlElement>(key))->ToElement()};
}

TiXmlElement* Serializer::getLastChild() const { return rootElement_->LastChild()->ToElement(); }

void Serializer::setAttribute(TiXmlElement* node, std::string_view key, std::string_view val) {
    node->SetAttribute(key, val);
}

void Serializer::serialize(std::string_view key, const signed char& data,
                           const SerializationTarget& target) {
    serialize(key, static_cast<int>(data), target);
}
void Serializer::serialize(std::string_view key, const char& data,
                           const SerializationTarget& target) {
    serialize(key, static_cast<int>(data), target);
}
void Serializer::serialize(std::string_view key, const unsigned char& data,
                           const SerializationTarget& target) {
    serialize(key, static_cast<unsigned int>(data), target);
}

void Serializer::writeFile() {
    try {
        doc_->SaveFile(getFileName().string());
    } catch (const TiXmlError& e) {
        throw SerializationException(e.what(), IVW_CONTEXT);
    }
}

void Serializer::writeFile(std::ostream& stream, bool format) {
    try {
        if (format) {
            TiXmlPrinter printer;
            doc_->Accept(&printer);
            stream << printer.Str();
        } else {
            stream << *doc_;
        }
    } catch (const TiXmlError& e) {
        throw SerializationException(e.what(), IVW_CONTEXT);
    }
}

}  // namespace inviwo
