/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <filesystem>

namespace inviwo {

Serializer::Serializer(const std::filesystem::path& fileName, std::string_view rootElement,
                       allocator_type alloc)
    : Serializer{fileName, rootElement, SerializeConstants::InviwoWorkspaceVersion, alloc} {}

Serializer::Serializer(const std::filesystem::path& fileName, std::string_view rootElement,
                       int version, allocator_type alloc)
    : SerializeBase(fileName, alloc) {
    try {
        auto decl = alloc.new_object<TiXmlDeclaration>(SerializeConstants::XmlVersion, "UTF-8", "");
        doc_->LinkEndChild(decl);
        rootElement_ = alloc.new_object<TiXmlElement>(rootElement);
        auto& attr = rootElement_->AddAttribute(SerializeConstants::VersionAttribute);
        detail::formatTo(version, attr);
        doc_->LinkEndChild(rootElement_);

    } catch (const TiXmlError& e) {
        throw SerializationException(e.what());
    }
}

Serializer::Serializer(allocator_type alloc)
    : Serializer{std::filesystem::path{}, SerializeConstants::InviwoWorkspace, alloc} {}

Serializer::~Serializer() = default;

void Serializer::serialize(std::string_view key, const std::filesystem::path& path,
                           SerializationTarget target) {

    if (target == SerializationTarget::Attribute) {
        auto& attr = rootElement_->AddAttribute(key);
        attr = path.generic_string<char, std::char_traits<char>,
                                   std::pmr::polymorphic_allocator<char>>(attr.get_allocator());
    } else {
        auto nodeSwitch = switchToNewNode(key);
        auto& attr = rootElement_->AddAttribute(SerializeConstants::ContentAttribute);
        attr = path.generic_string<char, std::char_traits<char>,
                                   std::pmr::polymorphic_allocator<char>>(attr.get_allocator());
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

std::pmr::string& Serializer::addAttribute(TiXmlElement* node, std::string_view key) {
    return node->AddAttribute(key);
}

std::pmr::string& Serializer::addAttribute(std::string_view key) {
    return rootElement_->AddAttribute(key);
}

void Serializer::serialize(std::string_view key, const signed char& data,
                           SerializationTarget target) {
    serialize(key, static_cast<int>(data), target);
}
void Serializer::serialize(std::string_view key, const char& data, SerializationTarget target) {
    serialize(key, static_cast<int>(data), target);
}
void Serializer::serialize(std::string_view key, const unsigned char& data,
                           SerializationTarget target) {
    serialize(key, static_cast<unsigned int>(data), target);
}

void Serializer::serialize(std::string_view key, std::string_view data,
                           SerializationTarget target) {
    if (target == SerializationTarget::Attribute) {
        rootElement_->AddAttribute(key, data);
    } else {
        auto nodeSwitch = switchToNewNode(key);
        rootElement_->AddAttribute(SerializeConstants::ContentAttribute, data);
    }
}
void Serializer::serialize(std::string_view key, const std::string& data,
                           SerializationTarget target) {
    serialize(key, std::string_view{data}, target);
}
void Serializer::serialize(std::string_view key, const std::pmr::string& data,
                           SerializationTarget target) {
    serialize(key, std::string_view{data}, target);
}

void Serializer::writeFile() {
    try {
        doc_->SaveFile(getFileName().string());
    } catch (const TiXmlError& e) {
        throw SerializationException(e.what());
    }
}

void Serializer::writeFile(std::ostream& stream, bool format) {
    std::pmr::string xml{getAllocator()};
    xml.reserve(4096);
    write(xml, format);
    stream << xml;
}

void Serializer::write(std::pmr::string& xml, bool format) {
    try {
        TiXmlPrinter printer{xml, format ? TiXmlStreamPrint::No : TiXmlStreamPrint::Yes};
        doc_->Accept(&printer);
    } catch (const TiXmlError& e) {
        throw SerializationException(e.what());
    }
}

}  // namespace inviwo
