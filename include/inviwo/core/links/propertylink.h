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

#ifndef IVW_PROPERYLINK_H
#define IVW_PROPERYLINK_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>

namespace inviwo {

class Property;

class IVW_CORE_API PropertyLink : public Serializable {
public:
    PropertyLink();
    PropertyLink(Property* srcProperty, Property* dstProperty);
    PropertyLink(const PropertyLink&) = default;
    PropertyLink& operator=(const PropertyLink&) = default;
    virtual ~PropertyLink() = default;

    operator bool() const;

    Property* getSource() const { return src_; }
    Property* getDestination() const { return dst_; }
        
    bool involves(Processor* processor) const;
    bool involves(Property* property) const;

    virtual void serialize(Serializer& s) const;
    virtual void deserialize(Deserializer& d);

    friend bool IVW_CORE_API operator==(const PropertyLink& lhs, const PropertyLink& rhs);
    friend bool IVW_CORE_API operator<(const PropertyLink& lhs, const PropertyLink& rhs);

private:
    Property* src_;
    Property* dst_;
};

bool IVW_CORE_API operator==(const PropertyLink& lhs, const PropertyLink& rhs);
bool IVW_CORE_API operator!=(const PropertyLink& lhs, const PropertyLink& rhs);
bool IVW_CORE_API operator<(const PropertyLink& lhs, const PropertyLink& rhs);

}  // namespace

namespace std {

template <>
struct hash<inviwo::PropertyLink> {
    size_t operator()(const inviwo::PropertyLink& p) const {
        size_t h = 0;
        inviwo::util::hash_combine(h, p.getSource());
        inviwo::util::hash_combine(h, p.getDestination());
        return h;
    }
};

}  // namespace std

#endif  // IVW_PROPERYLINK_H
