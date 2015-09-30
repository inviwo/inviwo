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

#include <inviwo/core/properties/propertysemantics.h>

namespace inviwo {

PropertySemantics::PropertySemantics() : IvwSerializable(), semantic_("Default") {}
PropertySemantics::PropertySemantics(std::string semantic)
    : IvwSerializable(), semantic_(semantic) {}
PropertySemantics::PropertySemantics(const PropertySemantics& rhs)
    : IvwSerializable(), semantic_(rhs.semantic_) {};
PropertySemantics& PropertySemantics::operator=(const PropertySemantics& that) {
    if (this != &that) semantic_ = that.semantic_;

    return *this;
}

const std::string& PropertySemantics::getString() const { return semantic_; }

void PropertySemantics::serialize(IvwSerializer& s) const {
    s.serialize("semantics", semantic_, true);
}

void PropertySemantics::deserialize(IvwDeserializer& d) {
    d.deserialize("semantics", semantic_, true);
}

std::ostream& operator<<(std::ostream& os, const inviwo::PropertySemantics& obj) {
    os << obj.semantic_;
    return os;
}

const PropertySemantics PropertySemantics::Default("Default");
const PropertySemantics PropertySemantics::Color("Color");
const PropertySemantics PropertySemantics::LightPosition("LightPosition");
const PropertySemantics PropertySemantics::Multiline("Multiline");
const PropertySemantics PropertySemantics::TextEditor("TextEditor");
const PropertySemantics PropertySemantics::ImageEditor("ImageEditor");
const PropertySemantics PropertySemantics::ShaderEditor("ShaderEditor");

} // namespace