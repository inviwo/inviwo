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

PropertyLink::PropertyLink() : srcProperty_(NULL), dstProperty_(NULL) {}

PropertyLink::~PropertyLink() {}

PropertyLink::PropertyLink(Property* srcProperty, Property* destProperty)
    : srcProperty_(srcProperty), dstProperty_(destProperty) {}

void PropertyLink::serialize(IvwSerializer& s) const {
    s.serialize("SourceProperty", srcProperty_);
    s.serialize("DestinationProperty", dstProperty_);
}

void PropertyLink::deserialize(IvwDeserializer& d) {
    struct LError {
        LError() : error(false), data(){};
        bool error;
        SerializationException::SerializationExceptionData data;
    };
    LError src, dest;

    try {
        d.deserialize("SourceProperty", srcProperty_);
    } catch (SerializationException& e) {
        src.error = true;
        src.data = e.getData();
    }

    try {
        d.deserialize("DestinationProperty", dstProperty_);
    } catch (SerializationException& e) {
        dest.error = true;
        dest.data = e.getData();
    }

    if (src.error && dest.error) {
        throw SerializationException(
            "Could not create Property Link from " + src.data.nd.getDescription() + " to " +
                dest.data.nd.getDescription() + ". Source and destination properties not found.",
            "PropertyLink");

    } else if (src.error) {
        throw SerializationException(
            "Could not create Property Link from " + src.data.nd.getDescription() + " to " +
                joinString(dstProperty_->getPath(), ".") + "\". Source property not found.",
            "PropertyLink");
    } else if (dest.error) {
        throw SerializationException(
            "Could not create Property Link from \"" + joinString(srcProperty_->getPath(), ".") +
                "\" to " + dest.data.nd.getDescription() + ". Destination property not found.",
            "PropertyLink");
    }
}

bool operator==(const PropertyLink& lhs, const PropertyLink& rhs) {
    return lhs.srcProperty_ == rhs.srcProperty_ && lhs.dstProperty_ == rhs.dstProperty_;
}
bool operator!=(const PropertyLink& lhs, const PropertyLink& rhs) { return !operator==(lhs, rhs); }

bool operator<(const PropertyLink& lhs, const PropertyLink& rhs) {
    if (lhs.srcProperty_ != rhs.srcProperty_) {
        return lhs.srcProperty_ < rhs.srcProperty_;
    } else {
        return lhs.dstProperty_ < rhs.dstProperty_;
    }
}

}  // namespace
