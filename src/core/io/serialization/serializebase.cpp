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
#include <inviwo/core/io/serialization/serializebase.h>
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/common/inviwo.h>


namespace inviwo {

SerializeBase::ReferenceDataContainer::ReferenceDataContainer() {
    referenceCount_ = 0;
}

SerializeBase::ReferenceDataContainer::~ReferenceDataContainer() {
}

size_t SerializeBase::ReferenceDataContainer::insert(const void* data, TxElement* node, bool isPointer) {
    SerializeBase::ReferenceData refData;
    refData.node_ = node;
    refData.isPointer_ = isPointer;
    referenceMap_.insert(RefDataPair(data, refData));
    return referenceMap_.count(data);
}


void SerializeBase::ReferenceDataContainer::setReferenceAttributes() {
    std::pair<RefMap::const_iterator, RefMap::const_iterator> sameKeys;

    // Loop over all different key valus.
    for (RefMap::const_iterator uniqueKey = referenceMap_.begin();
         uniqueKey != referenceMap_.end();
         uniqueKey = referenceMap_.upper_bound(uniqueKey->first)) {
        sameKeys = referenceMap_.equal_range(uniqueKey->first);

        if (std::distance(sameKeys.first, sameKeys.second)<=1) continue;

        std::stringstream ss;
        ss<<"ref";
        ss<<referenceCount_;

        // Loop over all items with the same key as uniqueKey.
        for (RefMap::const_iterator item = sameKeys.first;
             item != sameKeys.second; ++item) {
            if (item->second.isPointer_)
                item->second.node_->SetAttribute(SerializeConstants::RefAttribute, ss.str());
            else
                item->second.node_->SetAttribute(SerializeConstants::IDAttribute, ss.str());
        }

        referenceCount_++;
    }
}

size_t SerializeBase::ReferenceDataContainer::find(const void* data) {
    return referenceMap_.count(data);
}

void* SerializeBase::ReferenceDataContainer::find(const std::string& type, const std::string& reference_or_id) {
    void* data = nullptr;

    if (reference_or_id.empty())
        return data;

    for (auto& elem : referenceMap_) {
        std::string type_attrib("");
        std::string ref_attrib("");
        std::string id_attrib("");
        elem.second.node_->GetAttribute(SerializeConstants::TypeAttribute, &type_attrib, false);
        elem.second.node_->GetAttribute(SerializeConstants::RefAttribute, &ref_attrib, false);
        elem.second.node_->GetAttribute(SerializeConstants::IDAttribute, &id_attrib, false);

        if (type_attrib == type && (ref_attrib == reference_or_id || id_attrib == reference_or_id)) {
            data = const_cast<void*>(elem.first);
            break;
        }
    }

    return data;
}

TxElement* SerializeBase::ReferenceDataContainer::nodeCopy(const void* data) {
    std::pair<RefMap::iterator, RefMap::iterator> pIt;
    std::vector<ReferenceData> nodes;
    TxElement* nodeCopy = nullptr;
    pIt = referenceMap_.equal_range(data);

    for (RefMap::iterator mIt = pIt.first; mIt != pIt.second; ++mIt) {
        nodeCopy = mIt->second.node_->Clone()->ToElement();

        if (nodeCopy) {
            nodeCopy->Clear();
            break;
        }
    }

    return nodeCopy;
}

SerializeBase::SerializeBase(bool allowReference/*=true*/)
    : allowRef_(allowReference)
    , retrieveChild_(true) {
}

SerializeBase::SerializeBase(SerializeBase& s, bool allowReference)
    : fileName_(s.fileName_)
    , doc_(s.fileName_)
    , allowRef_(allowReference)
    , retrieveChild_(true) {
}

SerializeBase::SerializeBase(std::string fileName, bool allowReference)
    : fileName_(fileName)
    , doc_(fileName)
    , allowRef_(allowReference)
    , retrieveChild_(true) {
}

SerializeBase::SerializeBase(std::istream& stream, const std::string& path, bool allowReference)
    : fileName_(path)
    , allowRef_(allowReference)
    , retrieveChild_(true) {
    stream >> doc_;
}

SerializeBase::~SerializeBase() {
}


const std::string& SerializeBase::getFileName() const {
    return fileName_;
}

bool SerializeBase::isPrimitiveType(const std::type_info& type) const {
    if (type == typeid(bool)
        || type == typeid(char)
        || type == typeid(int)
        || type == typeid(signed int)
        || type == typeid(unsigned int)
        || type == typeid(float)
        || type == typeid(double)
        || type == typeid(long double)
        || type == typeid(std::string))
        return true;

    return false;
}

std::string SerializeBase::nodeToString(const TxElement& node) {
    try {
        TiXmlPrinter printer;
        printer.SetIndent("    ");
        node.Accept(&printer);
        return printer.CStr();
    } catch (TxException&) {
        return "No valid root node";
    }
}

NodeSwitch::NodeSwitch(SerializeBase& serializer, TxElement* node, bool retrieveChild)
    : serializer_(serializer)
    , storedNode_(serializer_.rootElement_)
    , storedRetrieveChild_(serializer_.retrieveChild_) {

    serializer_.rootElement_ = node;
    serializer_.retrieveChild_ = retrieveChild;
}

NodeSwitch::NodeSwitch(SerializeBase& serializer, const std::string& key, bool retrieveChild)
    : serializer_(serializer)
    , storedNode_(serializer_.rootElement_)
    , storedRetrieveChild_(serializer_.retrieveChild_) {

    serializer_.rootElement_ = serializer_.retrieveChild_
                                   ? serializer_.rootElement_->FirstChildElement(key, false)
                                   : serializer_.rootElement_;

    serializer_.retrieveChild_ = retrieveChild;
}

NodeSwitch::~NodeSwitch() {
    serializer_.rootElement_ = storedNode_;
    serializer_.retrieveChild_ = storedRetrieveChild_;
}

NodeSwitch::operator bool() const { return serializer_.rootElement_ != nullptr;}

} //namespace
