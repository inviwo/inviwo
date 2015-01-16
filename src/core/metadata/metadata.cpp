/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <inviwo/core/metadata/metadata.h>

namespace inviwo {

MetaData::MetaData() {}
MetaData::MetaData(const MetaData& rhs) {};
MetaData& MetaData::operator=(const MetaData& that) {
    return *this;
}
MetaData::~MetaData() {}

std::string MetaData::getClassIdentifier() const {
    return "MetaData";
}

MetaData* MetaData::clone() const {
    return new MetaData(*this);
}

void MetaData::serialize(IvwSerializer& s) const {
    IVW_UNUSED_PARAM(s);
}

void MetaData::deserialize(IvwDeserializer& d) {
    IVW_UNUSED_PARAM(d);
}

bool MetaData::equal(const MetaData& rhs) const {
    return false;
}

bool operator==(const MetaData& lhs, const MetaData& rhs) {
    return lhs.equal(rhs);
}

bool operator!=(const MetaData& lhs, const MetaData& rhs) {
    return !operator==(lhs, rhs);
}

} // namespace
