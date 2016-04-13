/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#pragma warning(disable : 4251)
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/util/exception.h>

namespace inviwo {

Serializer::Serializer(const std::string& fileName, bool allowReference)
    : SerializeBase(fileName, allowReference) {
    try {
        auto decl = util::make_unique<TxDeclaration>(SerializeConstants::XmlVersion, "", "");
        doc_.LinkEndChild(decl.get());
        rootElement_ = new TxElement(SerializeConstants::InviwoTreedata);
        rootElement_->SetAttribute(SerializeConstants::VersionAttribute,
                                   SerializeConstants::InviwoVersion);
        doc_.LinkEndChild(rootElement_);
        auto comment = util::make_unique<TxComment>();
        comment->SetValue(SerializeConstants::EditComment.c_str());
        rootElement_->LinkEndChild(comment.get());

    } catch (TxException& e) {
        throw SerializationException(e.what(), IvwContext);
    }
}

Serializer::~Serializer() { delete rootElement_; }

void Serializer::serialize(const std::string& key, const Serializable& sObj) {
    auto newNode = util::make_unique<TxElement>(key);
    rootElement_->LinkEndChild(newNode.get());
    NodeSwitch nodeSwitch(*this, newNode.get());
    sObj.serialize(*this);
}

void Serializer::serialize(const std::string& key, const signed char& data,
                           const SerializationTarget& target) {
    serialize(key, static_cast<int>(data), target);
}
void Serializer::serialize(const std::string& key, const char& data,
                           const SerializationTarget& target) {
    serialize(key, static_cast<int>(data), target);
}
void Serializer::serialize(const std::string& key, const unsigned char& data,
                           const SerializationTarget& target) {
    serialize(key, static_cast<unsigned int>(data), target);
}

void Serializer::writeFile() {
    try {
        refDataContainer_.setReferenceAttributes();
        doc_.SaveFile(getFileName());
    } catch (TxException& e) {
        throw SerializationException(e.what(), IvwContext);
    }
}

void Serializer::writeFile(std::ostream& stream) {
    try {
        refDataContainer_.setReferenceAttributes();
        stream << doc_;
    } catch (TxException& e) {
        throw SerializationException(e.what(), IvwContext);
    }
}

}  // namespace
