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
#include <inviwo/core/io/serialization/serializeconstants.h>

namespace inviwo {

const std::string SerializeConstants::XmlVersion="1.0";
const std::string SerializeConstants::InviwoTreedata="InviwoTreeData";
const std::string SerializeConstants::InviwoVersion="1.0";
const std::string SerializeConstants::NetworkVersion="1.0";
const std::string SerializeConstants::Version="version";
const std::string SerializeConstants::EditComment=" Don't edit the following code ";
const std::string SerializeConstants::IDAttribute="id";
const std::string SerializeConstants::RefAttribute="reference";
const std::string SerializeConstants::VersionAttribute="version";
const std::string SerializeConstants::ContentAttribute="content";
const std::string SerializeConstants::TypeAttribute="type";
const std::string SerializeConstants::KeyAttribute="key";

const std::string SerializeConstants::VectorAttributes[] = {"x", "y", "z", "w"};

const std::string SerializeConstants::PropertyAttribute1="identifier";
const std::string SerializeConstants::PropertyAttribute2="displayName";
const std::string SerializeConstants::ProcessorAttribute1="identifier";
const std::string SerializeConstants::ProcessorAttribute2="displayName";

bool SerializeConstants::isReversvedAttribute(const std::string key) {
    if (key == PropertyAttribute1
        || key == PropertyAttribute2
        || key == ProcessorAttribute1
        || key == ProcessorAttribute2)
        return true;

    return false;
}

} //namespace
