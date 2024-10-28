/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/util/filedialogstate.h>

namespace inviwo {

const std::string DirectoryProperty::classIdentifier = "org.inviwo.DirectoryProperty";
std::string_view DirectoryProperty::getClassIdentifier() const { return classIdentifier; }

DirectoryProperty::DirectoryProperty(std::string_view identifier, std::string_view displayName,
                                     Document help, std::string_view value,
                                     std::string_view contentType,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : FileProperty(identifier, displayName, std::move(help), value, contentType, invalidationLevel,
                   semantics) {
    setAcceptMode(AcceptMode::Open);
    setFileMode(FileMode::Directory);
}

DirectoryProperty::DirectoryProperty(std::string_view identifier, std::string_view displayName,
                                     std::string_view value, std::string_view contentType,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : DirectoryProperty(identifier, displayName, Document{}, value, contentType, invalidationLevel,
                        semantics) {}

DirectoryProperty::DirectoryProperty(const DirectoryProperty& rhs) = default;
DirectoryProperty* DirectoryProperty::clone() const { return new DirectoryProperty(*this); }

DirectoryProperty::~DirectoryProperty() = default;
std::string_view DirectoryProperty::getClassIdentifierForWidget() const {
    return FileProperty::getClassIdentifier();
}

}  // namespace inviwo
