/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/core/links/propertylink.h>

namespace inviwo {

PropertyLink::PropertyLink()
    : srcProperty_(NULL),
      dstProperty_(NULL) {
}

PropertyLink::~PropertyLink() {}

PropertyLink::PropertyLink(Property* srcProperty, Property* destProperty)
    : srcProperty_(srcProperty), dstProperty_(destProperty) {
}

void PropertyLink::serialize(IvwSerializer& s) const {
    std::vector<Property*> linkedProperties;
    linkedProperties.push_back(srcProperty_);
    linkedProperties.push_back(dstProperty_);
    s.serialize("Properties", linkedProperties, "Property");
}

void PropertyLink::deserialize(IvwDeserializer& d) {

    struct LinkError {
        typedef std::pair<std::string, SerializationException::SerializationExceptionData> LError;
        void handleError(SerializationException& error) {
            errors_.push_back(LError(error.getMessage(), error.getData()));
        }
        std::vector<LError> errors_; 
    };

    LinkError handle;

    std::vector<Property*> linkedProperties;
    DeserializationErrorHandle<LinkError> err(d, "Property", &handle, &LinkError::handleError);
    try {
        d.deserialize("Properties", linkedProperties, "Property");

        if (!handle.errors_.empty()) {
            std::string processorId("");
            TxElement* xprop = handle.errors_[0].second.node;
            if (xprop) {
                TxElement* xlist = xprop->Parent()->ToElement();
                if (xlist) {
                    TxElement* xproc = xlist->Parent()->ToElement();
                    processorId = xproc->GetAttributeOrDefault("identifier", "");
                }
            }
            throw SerializationException("Could not create link " + processorId,
                                         "PropertyLink");
        }

        srcProperty_ = linkedProperties[0];
        dstProperty_ = linkedProperties[1];
    } catch (SerializationException& error) {
        throw SerializationException("Could not create link of type " + error.getType(),
                                     "PropertyLink", error.getType());
    }
}

bool operator==(const PropertyLink& lhs, const PropertyLink& rhs) {
    return lhs.srcProperty_ == rhs.srcProperty_ && lhs.dstProperty_ == rhs.dstProperty_;
}
bool operator!=(const PropertyLink& lhs, const PropertyLink& rhs) {
    return !operator==(lhs, rhs);
}

bool  operator<(const PropertyLink& lhs, const PropertyLink& rhs) {
    if (lhs.srcProperty_ != rhs.srcProperty_) {
        return lhs.srcProperty_ < rhs.srcProperty_;
    } else {
        return lhs.dstProperty_ < rhs.dstProperty_;
    }
}

} // namespace

