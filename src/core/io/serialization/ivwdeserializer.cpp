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

#pragma warning(disable: 4251)
#include <inviwo/core/io/serialization/ivwdeserializer.h>
#include <inviwo/core/util/factory.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/io/serialization/ivwserializable.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/io/serialization/versionconverter.h>

namespace inviwo {

IvwDeserializer::IvwDeserializer(IvwDeserializer& s, bool allowReference)
    : IvwSerializeBase(s.getFileName(), allowReference) {
    doc_.LoadFile();
    rootElement_ = doc_.FirstChildElement();
    storeReferences(rootElement_);
}

IvwDeserializer::IvwDeserializer(std::string fileName, bool allowReference)
    : IvwSerializeBase(fileName, allowReference) {
    doc_.LoadFile();
    rootElement_ = doc_.FirstChildElement();
    storeReferences(rootElement_);
}

IvwDeserializer::IvwDeserializer(std::istream& stream, const std::string& path, bool allowReference)
    : IvwSerializeBase(stream, path, allowReference) {
    // Base streamed in the xml data. Get the first node.
    rootElement_ = doc_.FirstChildElement();
    storeReferences(rootElement_);
}

IvwDeserializer::~IvwDeserializer() {}

void IvwDeserializer::deserialize(const std::string& key, IvwSerializable& sObj) {
    try {
        NodeSwitch ns(*this, key);
        sObj.deserialize(*this);
    } catch (TxException&) {
    }
}

void IvwDeserializer::deserialize(const std::string& key, signed char& data,
                                  const bool asAttribute) {
    int val = data;
    deserialize(key, val, asAttribute);
    data = static_cast<char>(val);
}
void IvwDeserializer::deserialize(const std::string& key, char& data,
                                  const bool asAttribute) {
    int val = data;
    deserialize(key, val, asAttribute);
    data = static_cast<char>(val);
}
void IvwDeserializer::deserialize(const std::string& key, unsigned char& data,
                                  const bool asAttribute) {
    unsigned int val = data;
    deserialize(key, val, asAttribute);
    data = static_cast<unsigned char>(val);
}

void IvwDeserializer::convertVersion(VersionConverter* converter) {
    converter->convert(rootElement_);
    // Re-generate the reference table
    referenceLookup_.clear();
    storeReferences(doc_.FirstChildElement());
}

void IvwDeserializer::pushErrorHandler(BaseDeserializationErrorHandler* handler) {
    errorHandlers_.push_back(handler);
}

BaseDeserializationErrorHandler* IvwDeserializer::popErrorHandler() {
    BaseDeserializationErrorHandler* back = errorHandlers_.back();
    errorHandlers_.pop_back();
    return back;
}

void IvwDeserializer::handleError(SerializationException& e) {
    for (auto it = errorHandlers_.rbegin(); it != errorHandlers_.rend(); ++it) {
        if ((*it)->getKey() == e.getKey()) {
            (*it)->handleError(e);
            return;
        }
    }
    // If no error handler found:
    util::log(e.getContext(), e.getMessage(), LogLevel::Warn);
}

void IvwDeserializer::storeReferences(TxElement* node) {
    std::string id = node->GetAttributeOrDefault("id", "");
    if (id != "") {
        referenceLookup_[id] = node;
    }
    ticpp::Iterator<ticpp::Element> child;
    for (child = child.begin(node); child != child.end(); child++) {
        storeReferences(child.Get());
    }
}

}  // namespace
