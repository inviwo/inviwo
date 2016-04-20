/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertyowner.h>

namespace inviwo {

PropertyLink::PropertyLink() : src_(nullptr), dst_(nullptr) {}

PropertyLink::operator bool() const {
    return src_ && dst_;
}

PropertyLink::PropertyLink(Property* src, Property* dst) : src_(src), dst_(dst) {}

void PropertyLink::serialize(Serializer& s) const {
    s.serialize("SourceProperty", src_);
    s.serialize("DestinationProperty", dst_);
}

void PropertyLink::deserialize(Deserializer& d) {
    struct LinkError {
        LinkError() : error(false), data(){};
        bool error;
        SerializationException::SerializationExceptionData data;
    };
    LinkError srcError, dstError;

    try {
        d.deserialize("SourceProperty", src_);
    } catch (SerializationException& e) {
        srcError.error = true;
        srcError.data = e.getData();
    }

    try {
        d.deserialize("DestinationProperty", dst_);
    } catch (SerializationException& e) {
        dstError.error = true;
        dstError.data = e.getData();
    }

    if (srcError.error && dstError.error) {
        throw SerializationException("Could not create Property Link from " +
                                         srcError.data.nd.getDescription() + " to " +
                                         dstError.data.nd.getDescription() +
                                         ". Source and destination properties not found.",
                                     IvwContext, "PropertyLink");

    } else if (srcError.error) {
        throw SerializationException(
            "Could not create Property Link from " + srcError.data.nd.getDescription() + " to " +
                joinString(dst_->getPath(), ".") + "\". Source property not found.",
            IvwContext, "PropertyLink");
    } else if (dstError.error) {
        throw SerializationException(
            "Could not create Property Link from \"" + joinString(src_->getPath(), ".") + "\" to " +
                dstError.data.nd.getDescription() + ". Destination property not found.",
            IvwContext, "PropertyLink");
    }
}

bool PropertyLink::involves(Processor* processor) const {
    return src_->getOwner()->getProcessor() == processor ||
           dst_->getOwner()->getProcessor() == processor;
}
bool PropertyLink::involves(Property* property) const {
    return src_ == property || dst_ == property;
}

bool operator==(const PropertyLink& lhs, const PropertyLink& rhs) {
    return lhs.src_ == rhs.src_ && lhs.dst_ == rhs.dst_;
}
bool operator!=(const PropertyLink& lhs, const PropertyLink& rhs) { return !operator==(lhs, rhs); }

bool operator<(const PropertyLink& lhs, const PropertyLink& rhs) {
    if (lhs.src_ != rhs.src_) {
        return lhs.src_ < rhs.src_;
    } else {
        return lhs.dst_ < rhs.dst_;
    }
}

}  // namespace
