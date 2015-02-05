/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <inviwo/core/io/serialization/ivwserializer.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/io/serialization/ivwserializable.h>
#include <inviwo/core/util/factory.h>
#include <inviwo/core/util/exception.h>

namespace inviwo {

IvwSerializer::IvwSerializer(IvwSerializer& s, bool allowReference)
    : IvwSerializeBase(s.getFileName(), allowReference) {
    initialize();
}

IvwSerializer::IvwSerializer(const std::string& fileName, bool allowReference)
    : IvwSerializeBase(fileName, allowReference) {
    initialize();
}


IvwSerializer::~IvwSerializer() {
    delete rootElement_;
}

void IvwSerializer::initialize() {
    try {
        TxComment* comment;
        TxDeclaration* decl = new TxDeclaration(IvwSerializeConstants::XML_VERSION, "", "");
        doc_.LinkEndChild(decl);
        rootElement_ = new TxElement(IvwSerializeConstants::INVIWO_TREEDATA);
        rootElement_->SetAttribute(IvwSerializeConstants::VERSION_ATTRIBUTE,
                                   IvwSerializeConstants::INVIWO_VERSION);
        doc_.LinkEndChild(rootElement_);
        comment = new TxComment();
        comment->SetValue(IvwSerializeConstants::EDIT_COMMENT.c_str());
        rootElement_->LinkEndChild(comment);
        delete comment;
        delete decl;
    } catch (TxException& e) {
        throw SerializationException(e.what());
    }
}

void IvwSerializer::serialize(const std::string& key, const IvwSerializable& sObj) {
    TxElement* newNode = new TxElement(key);
    rootElement_->LinkEndChild(newNode);
    NodeSwitch tempNodeSwitch(*this, newNode);
    sObj.serialize(*this);
    delete newNode;
}

void IvwSerializer::serialize(const std::string& key,
                              const std::string& data,
                              const bool asAttribute) {
    if (asAttribute)
        rootElement_->SetAttribute(key, data);
    else
        serializePrimitives<std::string>(key, data);
}

void IvwSerializer::serialize(const std::string& key, const bool& data) {
    serializePrimitives<bool>(key, data);
}
void IvwSerializer::serialize(const std::string& key, const float& data) {
    serializePrimitives<float>(key, data);
}
void IvwSerializer::serialize(const std::string& key, const double& data) {
    serializePrimitives<double>(key, data);
}
void IvwSerializer::serialize(const std::string& key, const int& data) {
    serializePrimitives<int>(key, data);
}

void IvwSerializer::serialize(const std::string& key, const unsigned int& data) {
    serializePrimitives<unsigned int>(key, data);
}

void IvwSerializer::serialize(const std::string& key, const long& data) {
    serializePrimitives<long>(key, data);
}

void IvwSerializer::serialize(const std::string& key, const long long& data) {
    serializePrimitives<long long>(key, data);
}
void IvwSerializer::serialize(const std::string& key, const unsigned long long& data) {
    serializePrimitives<unsigned long long>(key, data);
}


void IvwSerializer::writeFile() {
    try {
        refDataContainer_.setReferenceAttributes();
        doc_.SaveFile(getFileName());
    } catch (TxException& e) {
        throw SerializationException(e.what());
    }
}

void IvwSerializer::writeFile(std::ostream& stream) {
    try {
        refDataContainer_.setReferenceAttributes();
        stream << doc_;
    } catch (TxException& e) {
        throw SerializationException(e.what());
    }
}


} //namespace
