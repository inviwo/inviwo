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

#include <inviwo/core/io/serialization/ivwdeserializer.h>
#include <inviwo/core/util/factory.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/io/serialization/ivwserializable.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/io/serialization/versionconverter.h>


namespace inviwo {

IvwDeserializer::IvwDeserializer(IvwDeserializer& s, bool allowReference)
    : IvwSerializeBase(s.getFileName(), allowReference) {
    registerFactories();

    try {
        readXMLData();
    } catch (AbortException& e) {
        throw SerializationException(e.what());
    }
}

IvwDeserializer::IvwDeserializer(std::string fileName, bool allowReference)
    : IvwSerializeBase(fileName, allowReference) {
    registerFactories();

    try {
        readXMLData();
    } catch (AbortException& e) {
        throw SerializationException(e.what());
    }
}

IvwDeserializer::IvwDeserializer(std::istream& stream, const std::string& path, bool allowReference)
    : IvwSerializeBase(stream, path, allowReference) {
    registerFactories();
    // Base streamed in the xml data. Get the first node.
    rootElement_ = doc_.FirstChildElement();

}


IvwDeserializer::~IvwDeserializer() {
}

void IvwDeserializer::deserialize(const std::string& key, IvwSerializable& sObj) {
    try {
        TxElement* keyNode;

        if (retrieveChild_)
            keyNode = rootElement_->FirstChildElement(key);
        else
            keyNode = rootElement_;

        NodeSwitch tempNodeSwitch(*this, keyNode);
        sObj.deserialize(*this);
    } catch (TxException&) {}
}

void IvwDeserializer::deserializeAttributes(const std::string& key, std::string& data) {
    try {
        rootElement_->GetAttribute(key, &data);
    } catch (TxException&) {}
}

void IvwDeserializer::deserializePrimitive(const std::string& key, std::string& data) {
    if (retrieveChild_) {
        rootElement_->FirstChildElement(key)->GetAttribute(
            IvwSerializeConstants::CONTENT_ATTRIBUTE, &data);
    } else
        rootElement_->GetAttribute(IvwSerializeConstants::CONTENT_ATTRIBUTE, &data);
}

void IvwDeserializer::deserialize(const std::string& key,
                                  std::string& data,
                                  const bool asAttribute) {
    if (asAttribute)
        deserializeAttributes(key, data);
    else {
        try {
            deserializePrimitive(key, data);
        } catch (TxException&) {
            //Try one more time to deserialize as string attribute (content attribute)
            try {
                deserializeAttributes(IvwSerializeConstants::CONTENT_ATTRIBUTE, data);
            }
            catch (TxException&) {}
        }
    }
}


void IvwDeserializer::deserialize(const std::string& key, bool& data) {
    deserializePrimitive<bool>(key, data);
}
void IvwDeserializer::deserialize(const std::string& key, float& data) {
    deserializePrimitive<float>(key, data);
}
void IvwDeserializer::deserialize(const std::string& key, double& data) {
    deserializePrimitive<double>(key, data);
}
void IvwDeserializer::deserialize(const std::string& key, signed char& data) {
    deserializePrimitive<signed char>(key, data);
}
void IvwDeserializer::deserialize(const std::string& key, unsigned char& data) {
    deserializePrimitive<unsigned char>(key, data);
}
void IvwDeserializer::deserialize(const std::string& key, char& data) {
    deserializePrimitive<char>(key, data);
}
void IvwDeserializer::deserialize(const std::string& key, short& data) {
    deserializePrimitive<short>(key, data);
}
void IvwDeserializer::deserialize(const std::string& key, unsigned short& data) {
    deserializePrimitive<unsigned short>(key, data);
}
void IvwDeserializer::deserialize(const std::string& key, int& data) {
    deserializePrimitive<int>(key, data);
}
void IvwDeserializer::deserialize(const std::string& key, unsigned int& data) {
    deserializePrimitive<unsigned int>(key, data);
}
void IvwDeserializer::deserialize(const std::string& key, long& data) {
    deserializePrimitive<long>(key, data);
}
void IvwDeserializer::deserialize(const std::string& key, long long& data) {
    deserializePrimitive<long long>(key, data);
}
void IvwDeserializer::deserialize(const std::string& key, unsigned long long& data) {
    deserializePrimitive<unsigned long long>(key, data);
}

void IvwDeserializer::readXMLData() {
    try {
        doc_.LoadFile();
        rootElement_ = doc_.FirstChildElement();
    } catch (TxException&) {
        std::stringstream ss;
        ss << "Error reading file: " << IvwSerializeBase::fileName_;
        throw (AbortException(ss.str()));
    }
}

void IvwDeserializer::convertVersion(VersionConverter* converter) {
    converter->convert(rootElement_);
}

} //namespace
