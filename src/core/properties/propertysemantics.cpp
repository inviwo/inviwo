/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2021 Inviwo Foundation
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
#include <inviwo/core/io/serialization/serialization.h>

#include <ostream>

namespace inviwo {

PropertySemantics::PropertySemantics() : semantic_("Default") {}
PropertySemantics::PropertySemantics(std::string semantic) : semantic_(std::move(semantic)) {}

const std::string& PropertySemantics::getString() const { return semantic_; }

void PropertySemantics::serialize(Serializer& s) const {
    s.serialize("semantics", semantic_, SerializationTarget::Attribute);
}

void PropertySemantics::deserialize(Deserializer& d) {
    d.deserialize("semantics", semantic_, SerializationTarget::Attribute);
}

const PropertySemantics PropertySemantics::Default("Default");
const PropertySemantics PropertySemantics::Text("Text");
const PropertySemantics PropertySemantics::SpinBox("SpinBox");
const PropertySemantics PropertySemantics::Color("Color");
const PropertySemantics PropertySemantics::LightPosition("LightPosition");
const PropertySemantics PropertySemantics::Multiline("Multiline");
const PropertySemantics PropertySemantics::TextEditor("TextEditor");
const PropertySemantics PropertySemantics::PythonEditor("PythonEditor");
const PropertySemantics PropertySemantics::ImageEditor("ImageEditor");
const PropertySemantics PropertySemantics::ShaderEditor("ShaderEditor");

std::ostream& operator<<(std::ostream& ss, const PropertySemantics& obj) {
    ss << obj.getString();
    return ss;
}

}  // namespace inviwo
