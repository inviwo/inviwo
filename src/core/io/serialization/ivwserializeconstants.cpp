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

#include <inviwo/core/io/serialization/ivwserializeconstants.h>

namespace inviwo {

const std::string IvwSerializeConstants::XML_VERSION="1.0";
const std::string IvwSerializeConstants::INVIWO_TREEDATA="InviwoTreeData";
const std::string IvwSerializeConstants::INVIWO_VERSION="1.0";
const std::string IvwSerializeConstants::NETWORK_VERSION="1.0";
const std::string IvwSerializeConstants::VERSION="version";
const std::string IvwSerializeConstants::EDIT_COMMENT=" Don't edit the following code ";
const std::string IvwSerializeConstants::ID_ATTRIBUTE="id";
const std::string IvwSerializeConstants::REF_ATTRIBUTE="reference";
const std::string IvwSerializeConstants::VERSION_ATTRIBUTE="version";
const std::string IvwSerializeConstants::CONTENT_ATTRIBUTE="content";
const std::string IvwSerializeConstants::TYPE_ATTRIBUTE="type";
const std::string IvwSerializeConstants::KEY_ATTRIBUTE="key";
const std::string IvwSerializeConstants::COLOR_R_ATTRIBUTE="r";
const std::string IvwSerializeConstants::COLOR_G_ATTRIBUTE="g";
const std::string IvwSerializeConstants::COLOR_B_ATTRIBUTE="b";
const std::string IvwSerializeConstants::COLOR_A_ATTRIBUTE="a";
const std::string IvwSerializeConstants::VECTOR_X_ATTRIBUTE="x";
const std::string IvwSerializeConstants::VECTOR_Y_ATTRIBUTE="y";
const std::string IvwSerializeConstants::VECTOR_Z_ATTRIBUTE="z";
const std::string IvwSerializeConstants::VECTOR_W_ATTRIBUTE="w";
const unsigned int IvwSerializeConstants::STRINGSTREAM_PRECISION=8;

const std::string IvwSerializeConstants::PROPERTY_ATTRIBUTE_1="identifier";
const std::string IvwSerializeConstants::PROPERTY_ATTRIBUTE_2="displayName";
const std::string IvwSerializeConstants::PROCESSOR_ATTRIBUTE_1="identifier";
const std::string IvwSerializeConstants::PROCESSOR_ATTRIBUTE_2="displayName";

bool IvwSerializeConstants::isReversvedAttribute(const std::string key) {
    if (key == PROPERTY_ATTRIBUTE_1
        || key == PROPERTY_ATTRIBUTE_2
        || key == PROCESSOR_ATTRIBUTE_1
        || key == PROCESSOR_ATTRIBUTE_2)
        return true;

    return false;
}

} //namespace
