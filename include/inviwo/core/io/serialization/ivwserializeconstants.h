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

#ifndef IVW_SERIALIZE_CONSTANTS_H
#define IVW_SERIALIZE_CONSTANTS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <string>

namespace inviwo {

class IVW_CORE_API IvwSerializeConstants {
public:
    static const std::string XML_VERSION;
    static const std::string INVIWO_TREEDATA;
    static const std::string INVIWO_VERSION;
    static const std::string NETWORK_VERSION;
    static const std::string VERSION;
    static const std::string EDIT_COMMENT;
    static const std::string ID_ATTRIBUTE;
    static const std::string REF_ATTRIBUTE;
    static const std::string VERSION_ATTRIBUTE;
    static const std::string CONTENT_ATTRIBUTE;
    static const std::string TYPE_ATTRIBUTE;
    static const std::string KEY_ATTRIBUTE;
    static const std::string COLOR_R_ATTRIBUTE;
    static const std::string COLOR_G_ATTRIBUTE;
    static const std::string COLOR_B_ATTRIBUTE;
    static const std::string COLOR_A_ATTRIBUTE;
    static const std::string VECTOR_X_ATTRIBUTE;
    static const std::string VECTOR_Y_ATTRIBUTE;
    static const std::string VECTOR_Z_ATTRIBUTE;
    static const std::string VECTOR_W_ATTRIBUTE;
    static const unsigned int STRINGSTREAM_PRECISION;

    static const std::string PROPERTY_ATTRIBUTE_1;
    static const std::string PROPERTY_ATTRIBUTE_2;

    static const std::string PROCESSOR_ATTRIBUTE_1;
    static const std::string PROCESSOR_ATTRIBUTE_2;

    static bool isReversvedAttribute(const std::string key);
};

} //namespace
#endif